 /*
 * Copyright 2020, ASUSTeK Inc.
 * All Rights Reserved.
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND ASUS GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */
 
/*
	feature implement:
	1. traditaional qos
	2. bandwdith limiter (also for guest network)
	3. facebook wifi     (already EOL in the end of 2017)

	NOTE:
	qos mark bit 8~31 : TrendMicro adaptive qos usage, so ASUS only can use bit 0~7 for different applications
	ex. Traditional qos / bandwidth limiter / Facebook wifi
*/

#include "rc.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#ifdef RTCONFIG_FBWIFI
#include <fbwifi.h>
#endif
#ifdef RTCONFIG_BWDPI
#include <bwdpi.h>
#endif

#ifdef RTCONFIG_PERMISSION_MANAGEMENT
#include <PMS_DBAPIs.h>
#endif

/*
	DEBUG DEFINE
	QOSDBG  : normal debug
	QOSLOG  : qos logmessage, we can get more information to trace memory leakage or crash issue
*/
#define QOS_DEBUG           "/tmp/QOS_DEBUG"
#define QOSDBG(fmt,args...) \
	if(f_exists(QOS_DEBUG) > 0) { \
		_dprintf("[QOS][%s:(%d)]"fmt, __FUNCTION__, __LINE__, ##args); \
	}

#define QOS_LOG             "/tmp/QOS_LOG"
#define QOSLOG(fmt,args...) \
	if(f_exists(QOS_LOG) > 0) { \
		char info[1024]; \
		snprintf(info, sizeof(info), "echo \"[QOS][%s:(%d)]"fmt"\" >> /tmp/QOS_LOG.log", __FUNCTION__, __LINE__, ##args); \
		system(info); \
	}

/**
 * Traditional QoS
 * ===============
 *
 * 1. If CLS_ACT is defined,
 *    IMQ0 is used to managed ingress traffic of WAN0,
 *    IMQ1 is used to managed ingress traffic of WAN1,
 *    IMQ2 is used to managed ingress traffic of WAN2, etc.
 *
 * 2. Handle 1 is used to manage output traffic of wan_unit.
 *    Handle 2 is used to managed ingress traffic of wan_unit,
 *    whether CLS_ACT is defined or not.  (wan_unit starts from zero)
 */

/* Protect /tmp/qos, /tmp/qos.* */
static char *qos_lock = "qos";
static char *qos_ipt_lock = "qos_ipt";

#define TMP_QOS	"/tmp/qos"
static const char *qosfn = TMP_QOS;
static const char *mangle_fn = "/tmp/mangle_rules";
#ifdef RTCONFIG_IPV6
static const char *mangle_fn_ipv6 = "/tmp/mangle_rules_ipv6";
#endif

static const unsigned int max_wire_speed = 10240000;

static unsigned int ipv6_qos_applied = 0;

int etable_flag = 0;
int manual_return = 0;

#ifdef RTCONFIG_AMAS_WGN
static void WGN_ifname(int i, int j, char *wl_if)
{
	if (nvram_get_int("re_mode") == 1) {
		char prefix[128] = {0};
		char tmp[128] = {0};
		snprintf(prefix, sizeof(prefix), "wl%d.%d_", i, j);
		sprintf(wl_if, "%s", nvram_safe_get(strcat_r(prefix, "ifname", tmp)));

		QOSDBG(" wl_if=%s, prefix=%s, tmp=%s\n", wl_if, prefix, tmp);
		return;
	}
	else {
		get_wlxy_ifname(i, j, wl_if);
		return;
	}
}

static void WGN_subnet(const char *wgn, char *net, int len)
{
	char *buf = NULL, *g = NULL, *p = NULL;
	char *wif = NULL, *sub = NULL;

	g = buf = strdup(nvram_safe_get("wgn_brif_rulelist"));
	while (g) {
	if ((p = strsep(&g, "<")) == NULL) break;
		if ((vstrsep(p, ">", &wif, &sub)) != 2) continue;
		if (!strcmp(wgn, wif)) {
			snprintf(net, len, "%s", sub);
			break;
		}
	}
	if (buf) free(buf);
	QOSDBG(" wgn=%s, net=%s, sub=%s\n", wgn, net, sub);
}

static void add_iptables_AMAS_WGN(FILE *fn, const char *action)
{
	/* Setup guest network's ebtables rules */
	int  guest_mark = GUEST_INIT_MARKNUM;
	char wl[128] = {0}, wlv[128] = {0}, tmp[128] = {0}, *next = NULL, *next2 = NULL;
	char prefix[32] = {0};
	char mssid_mark[4] = {0};
	char wl_ifname[IFNAMSIZ] = {0};
	char *wl_if = wl_ifname;
	int  i = 0;
	int  j = 1;
	char *wgn = NULL;
	char net[20] = {0};

	/*
	example:
	iptables -t mangle -A PREROUTING -s 192.168.102.1/24 -j MARK --set-mark 0xa
	iptables -t mangle -A PREROUTING -s 192.168.102.1/24 -j RETURN
	*/

	foreach(wl, nvram_safe_get("wl_ifnames"), next) {
		SKIP_ABSENT_BAND_AND_INC_UNIT(i);
		snprintf(prefix, sizeof(prefix), "wl%d_", i);
		foreach(wlv, nvram_safe_get(strcat_r(prefix, "vifnames", tmp)), next2) {

			if(nvram_get_int(strcat_r(wlv, "_bss_enabled", tmp)) && 
			   nvram_get_int(strcat_r(wlv, "_bw_enabled" , tmp))) {
				wgn = nvram_safe_get(strcat_r(wlv, "_brif", tmp));
				WGN_subnet(wgn, net, sizeof(net));
				snprintf(mssid_mark, sizeof(mssid_mark), "%d", guest_mark);
				if (!strcmp(net, "")) continue;
				fprintf(fn, "-A PREROUTING -s %s -j %s %s\n", net, action, mssid_mark);
				fprintf(fn, "-A PREROUTING -s %s -j RETURN\n", net);
				guest_mark++;
			} //bss_enabled
			j++;
		}
		i++; j = 1;
	}
}
#endif

/* In load-balance mode, redirect TX of each WAN interface to
 * imq0 interface and limit TX speed of imq0 interface instead
 * of limiting it on each WAN interface.
 */
#define BWLIT_IMQ_ID	0

/*
	ip / mac / ip-range status
*/
enum {
	TYPE_UNKNOWN = -1,
	TYPE_IP = 0,
	TYPE_MAC,
	TYPE_IPRANGE,
	TYPE_GROUP
};

/*
	isIPnum : 0~255
*/
static int isIPnum(char *ip)
{
	int sum = -1;
	int finish = 0;

	if (ip == NULL || *ip == '\0')
		goto end;

	while (*ip != '\0')
	{
		/* character : 0~9 */
		if (*ip < '0' || *ip > '9')
			goto end;

		/* sum : 0~255 */
		sum = (sum * 10 + (*ip - '0'));
		if (sum > 255)
			goto end;

		ip++;
	}

	finish = 1;

end:
	if (finish == 0) sum = -1;
	return sum;
}

/*
	isSubnet : x.x.x.0/24 ~ x.x.x.0/32
*/
static int isSubnet(char *sub)
{
	int mask = safe_atoi(sub);

	if (mask < 24 || mask > 32)
		return 0;
	else
		return mask;
}

/*
	ip_range_checker:
	1. 192.168.1.*    = 192.168.1.1-254 (subnet)
	2. 192.168.1.0/24 = 192.168.1.1-254 (subnet)
	3. 192.168.1.10-20                  (short)
*/
static int ip_range_checker(char *old, char *new, int len)
{
	int ret = 0;
	int len_to_dot = 0, len_total = 0;
	int len_to_line = 0;
	char *p = NULL, *g = NULL, *buf = NULL;
	char a[4];
	char head[16];
	char host[16];
	int mask = 0;
	int mask_t = 0, mask_addr = 0;
	int host_start = 0, host_end = 0;
	struct in_addr inet_src, inet_dst;
	char *start = NULL, *end = NULL;

	memset(new, 0, len);
	memset(head, 0, sizeof(head));
	QOSLOG("old=%s", old);

	/* check fail case */
	g = buf = strdup(old);
	while ((p = strchr(g, '.')) != NULL) {
		len_to_dot = p - g;
		len_total += len_to_dot + 1;
		memset(a, 0, sizeof(a));
		strncpy(a, g, len_to_dot);

		// validate value is valid IP num
		if (isIPnum(a) == -1) {
			QOSLOG("fail case : a=%s, len_total=%d", a, len_total);
			goto end;
		}
		g += len_to_dot + 1;
	}

	/* copy head */
	strncpy(head, old, len_total);

	/* case1 : x.x.x.0/24 subnet */
	if (*g == '*') {
		snprintf(new, len, "%s1-%s254", head, head);
		ret = 1;
		goto end;
	}

	/* case2 : IP subnetting */
	p = NULL;
	p = strchr(g, '/');
	if (p != NULL) {
		len_to_line = p - g;
		memset(a, 0, sizeof(a));
		strncpy(a, g, len_to_line);

		// validate value is valid IP num
		if (isIPnum(a) == -1) {
			QOSLOG("case 2: p+=%s, g=%s, a=%s", p+1, g, a);
			goto end;
		}

		// get mask and mask_addr
		g += len_to_dot + 1;
		mask = isSubnet(p+1);
		if (mask == 0) goto end;
		snprintf(host, sizeof(host), "%s%s", head, a);
		mask_t = ntohl(inet_addr(host));
		mask_addr = mask_t & (0xffffffff & (0xffffffff << (32 - mask)));
		host_start = mask_addr + 1;
		host_end = mask_addr + (0xffffffff & ~(0xffffffff << (32 - mask))) - 1;
		inet_src.s_addr = htonl(host_start);
		inet_dst.s_addr = htonl(host_end);

		QOSLOG("case 2: mask=%d, mask_addr=%x, host_start=%x, host_end=%x", mask, mask_addr, host_start, host_end);

		start = inet_ntoa(inet_src);
		strncpy(new, start, strlen(start));
		strncpy(new + strlen(start), "-", 1);
		end = inet_ntoa(inet_dst);
		strncpy(new + strlen(start) + 1, end, strlen(end));
		ret = 1;
		goto end;
	}

	/* case3 : find minus in tail */
	p = NULL;
	p = strchr(g, '-');
	if (p != NULL) {
		len_to_line = p - g;
		memset(a, 0, sizeof(a));
		strncpy(a, g, len_to_line);

		// validate value is valid IP num
		if (isIPnum(a) == -1) {
			QOSLOG("case 3: p+=%s, g=%s, a=%s", p+1, g, a);
			goto end;
		}

		snprintf(new, len, "%s%s-%s%s", head, a, head, (p+1));
		ret = 1;
		goto end;
	}

end:
	if (buf) free(buf);

	QOSLOG("new=%s", new);
	return ret;
}

/*
	address_format_checker:
	1. unknown
	2. ip
	3. mac
	4. ip-range
*/
static void address_format_checker(int *type, char *old, char *new, int len)
{
	char *g = NULL, *buf = NULL;
	int s[6]; // strip mac address
	int is_ip = 0;
	int is_mac = 0;
	int is_range = 0;
	int is_group = 0;

	memset(s, 0, sizeof(s));

	// group format
	if (strstr(old, "@") != NULL) {
		is_group = 1;
		goto end;
	}

	// mac format
	g = buf = strdup(old);
	if (sscanf(g, "%02X:%02X:%02X:%02X:%02X:%02X",&s[0],&s[1],&s[2],&s[3],&s[4],&s[5]) == 6) {
		is_mac = 1;
		goto end;
	}

	// ip format
	g = buf;
	if (illegal_ipv4_address(g) == 0) {
		is_ip = 1;
		goto end;
	}

	// ip-range format
	g = buf;
	if (ip_range_checker(g, new, len) == 1) {
		is_range = 1;
		goto end;
	}

end:
	if (buf) free(buf);
	if (is_ip == 1) {
		*type = TYPE_IP;
		strncpy(new, old, len);
	}
	else if (is_mac == 1) {
		*type = TYPE_MAC;
		strncpy(new, old, len);
	}
	else if (is_range == 1) {
		*type = TYPE_IPRANGE;
	}
	else if (is_group == 1) {
		*type = TYPE_GROUP;
		strlcpy(new, old, len);
	}
	else {
		*type = TYPE_UNKNOWN;
		strncpy(new, "", len);
	}
	QOSLOG("is_ip=%d, is_mac=%d, is_range=%d, is_group = %d, type=%d, new=%s", is_ip, is_mac, is_range, is_group, *type, new);
}

static unsigned calc(unsigned bw, unsigned pct)
{
	unsigned n = ((unsigned long)bw * pct) / 100;
	return (n < 2) ? 2 : n;
}

void del_EbtablesRules(void)
{
	/* Flush all rules in nat table of ebtable*/
	eval("ebtables", "-t", "nat", "-F");
	etable_flag = 0;
}

#ifdef CONFIG_BCMWL5 // TODO: it is only for the case, eth0 as wan, vlanx as lan

#if defined(RTCONFIG_FBWIFI)
static void set_fbwifi_mark(void)
{
	int band, j, max_mssid;
	char mark[16], inv_mask[16];	/* for ebtables mark, inverse mask */
	char wl_ifname[IFNAMSIZ] = "", *wl_if = wl_ifname;
	char *fbwifi_iface[3] = { "fbwifi_2g", "fbwifi_5g", "fbwifi_5g_2" };

	if (!nvram_match("fbwifi_enable","on"))
		return;

	snprintf(mark, sizeof(mark), "0x%x", FBWIFI_MARK_SET(1));
	snprintf(inv_mask, sizeof(inv_mask), "0x%x", FBWIFI_MARK_INV_MASK);
	for (band = 0; band < min(MAX_NR_WL_IF, ARRAYSIZE(fbwifi_iface)); ++band) {
		SKIP_ABSENT_BAND(band);

		if (nvram_match(fbwifi_iface[band], "off"))
			continue;

		max_mssid = num_of_mssid_support(band);
		for (j = 1; j <= max_mssid; ++j) {
			get_wlxy_ifname(band, j, wl_if);
			eval("ebtables", "-D", "INPUT", "-i", wl_if, "-j", "mark", "--mark-and", inv_mask, "--mark-target", "CONTINUE");
			eval("ebtables", "-D", "INPUT", "-i", wl_if, "-j", "mark", "--mark-or", mark, "--mark-target", "ACCEPT");
		}

		if (sscanf(nvram_safe_get(fbwifi_iface[band]), "wl%*d.%d", &j) != 1)
			continue;

		get_wlxy_ifname(band, j, wl_if);
		eval("ebtables", "-A", "INPUT", "-i", wl_if, "-j", "mark", "--mark-and", inv_mask, "--mark-target", "CONTINUE");
		eval("ebtables", "-A", "INPUT", "-i", wl_if, "-j", "mark", "--mark-or", mark, "--mark-target", "ACCEPT");
	}
}
#else
static inline void set_fbwifi_mark(void) { };
#endif

void add_EbtablesRules(void)
{
	if(etable_flag == 1) return;
	char *nv, *p, *g;
	nv = g = strdup(nvram_safe_get("wl_ifnames"));
	if(nv){
		while ((p = strsep(&g, " ")) != NULL){
			SKIP_ABSENT_FAKE_IFACE(p);
			QOSLOG("p=%s", p);
			eval("ebtables", "-t", "nat", "-A", "PREROUTING", "-i", p, "-j", "mark", "--mark-or", "6", "--mark-target", "ACCEPT");
			eval("ebtables", "-t", "nat", "-A", "POSTROUTING", "-o", p, "-j", "mark", "--mark-or", "6", "--mark-target", "ACCEPT");
		}
		free(nv);
	}

	// for MultiSSID
	int UnitNum = 2; 	// wl0.x, wl1.x
	int GuestNum = MAX_NO_MSSID - 1;	// wlx.0, wlx.1, wlx.2
	char mssid_if[32];
	char mssid_enable[32];
	int i, j;
	for( i = 0; i < UnitNum; i++){
		for( j = 1; j <= GuestNum; j++ ){
			snprintf(mssid_if, sizeof(mssid_if), "wl%d.%d", i, j);
			snprintf(mssid_enable, sizeof(mssid_enable), "%s_bss_enabled", mssid_if);
			QOSLOG("mssid_enable=%s", mssid_enable);
			if(!strcmp(nvram_safe_get(mssid_enable), "1")){
				eval("ebtables", "-t", "nat", "-A", "PREROUTING", "-i", mssid_if, "-j", "mark", "--mark-or", "6", "--mark-target", "ACCEPT");
				eval("ebtables", "-t", "nat", "-A", "POSTROUTING", "-o", mssid_if, "-j", "mark", "--mark-or", "6", "--mark-target", "ACCEPT");
			}
		}
	}

 #ifdef RTCONFIG_FBWIFI
	if(sw_mode() == SW_MODE_AP){
		set_fbwifi_mark();
	}
 #endif

	etable_flag = 1;
}
#endif

void remove_iptables_rules(int ipv6, const char *filename, char *flush_chains, char *skip_chains)
{
	FILE *fn;
	char cmd[1024];
	int len;
	char word[256], *next_word;
	const char *program = "iptables";

	if(ipv6)
		program = "ip6tables";

	if((fn = fopen(filename, "r")) != NULL){
		len = 0;
		while(fgets(cmd +len, sizeof(cmd)-len, fn) != NULL){
			if(len == 0) { /* no table name */
				if(cmd[0] == '*'){
					char table[16], *t = table;
					char *p = cmd;
					while(!isspace(*++p))
						*t++ = *p;
					*t++ = '\0';
					len = sprintf(cmd, "%s -t %s ", program, table);
				}
			} else if (cmd[len] == ':') { /* find chains */
				if(flush_chains != NULL && flush_chains[0] != '\0'){
					foreach(word, flush_chains, next_word){ /* flush this chain */
						if(strcmp(cmd +len +1, word) == 0){
							snprintf(cmd + len, sizeof(cmd)-len, "-F %s", word);
//							cprintf("## system(%s)\n", cmd);
							system(cmd);
							break;
						}
					}
				}
			} else if (strncmp("-A ", cmd +len, 3) == 0){
				int skip;
				skip = 0;
				if(flush_chains != NULL && flush_chains[0] != '\0'){
					foreach(word, flush_chains, next_word){
						if(strcmp(cmd +len +3, word) == 0) {
							skip = 1;
							break;
						}
					}
				}
				if(skip == 0 && skip_chains != NULL && skip_chains[0] != '\0') {
					foreach(word, skip_chains, next_word){
						if(strcmp(cmd +len +3, word) == 0){
							skip = 1;
							break;
						}
					}
				}
				if(skip == 0){
					int end;
					cmd[len + 1] = 'D';
					end = strlen(cmd);
					while(--end >= 0 && (cmd[end] == '\r' || cmd[end] == '\n'))
						cmd[end] = '\0';
//					cprintf("## system(%s)\n", cmd);
					system(cmd);
				}
			}
		}
		fclose(fn);
	}
}

void add_EbtablesRules_BW()
{
	if(etable_flag == 1) return;

	/* Setup guest network's ebtables rules */
	int  guest_mark = GUEST_INIT_MARKNUM;
	char wl[128], wlv[128], tmp[128], *next, *next2;
	char prefix[32];
	char mssid_mark[4];
	char wl_ifname[IFNAMSIZ];
	char *wl_if = wl_ifname;
	int  i = 0;
	int  j = 1;
	foreach(wl, nvram_safe_get("wl_ifnames"), next) {
		SKIP_ABSENT_BAND_AND_INC_UNIT(i);
		snprintf(prefix, sizeof(prefix), "wl%d_", i);
		foreach(wlv, nvram_safe_get(strcat_r(prefix, "vifnames", tmp)), next2) {

			if(nvram_get_int(strcat_r(wlv, "_bss_enabled", tmp)) && 
			   nvram_get_int(strcat_r(wlv, "_bw_enabled" , tmp))) {
#ifdef RTCONFIG_AMAS_WGN
				WGN_ifname(i, j, wl_if);
#else
				get_wlxy_ifname(i, j, wl_if);
#endif
				if (!strcmp(wl_if, "")) continue;
				snprintf(mssid_mark, sizeof(mssid_mark), "%d", guest_mark);
				eval("ebtables", "-t", "nat", "-D", "PREROUTING",  "-i", wl_if, "-j", "mark", "--set-mark", mssid_mark, "--mark-target", "ACCEPT");
				eval("ebtables", "-t", "nat", "-D", "POSTROUTING", "-o", wl_if, "-j", "mark", "--set-mark", mssid_mark, "--mark-target", "ACCEPT");
				eval("ebtables", "-t", "nat", "-A", "PREROUTING",  "-i", wl_if, "-j", "mark", "--set-mark", mssid_mark, "--mark-target", "ACCEPT");
				eval("ebtables", "-t", "nat", "-A", "POSTROUTING", "-o", wl_if, "-j", "mark", "--set-mark", mssid_mark, "--mark-target", "ACCEPT");
				guest_mark++;
			} //bss_enabled
			j++;
		}
		i++; j = 1;
	}

	etable_flag = 1;
	QOSDBG("[BWLIT_GUEST] Create ebtables rules done.\n");
}

void del_iQosRules(void)
{
	int lock;
#ifdef CLS_ACT
	int u;
	char imq_if[IFNAMSIZ];

	for (u = WAN_UNIT_FIRST; u < WAN_UNIT_MAX; ++u) {
		snprintf(imq_if, sizeof(imq_if), "imq%d", u);
		eval("ip", "link", "set", imq_if, "down");
	}
#endif

	lock = file_lock(qos_ipt_lock);
#ifdef CONFIG_BCMWL5 // TODO: it is only for the case, eth0 as wan, vlanx as lan
	del_EbtablesRules(); // flush ebtables nat table
#endif

	remove_iptables_rules(0, mangle_fn, "QOSO0 QOSO1", NULL);
	unlink(mangle_fn);
#ifdef RTCONFIG_IPV6
	if (ipv6_qos_applied) {
		remove_iptables_rules(1, mangle_fn_ipv6, "QOSO0 QOSO1", NULL);
		unlink(mangle_fn_ipv6);
		ipv6_qos_applied = 0;
	}
#endif	/* RTCONFIG_IPV6 */
	file_unlock(lock);
}

static int add_qos_rules(char *pcWANIF)
{
	FILE *fn;
#ifdef RTCONFIG_IPV6
	FILE *fn_ipv6 = NULL;
#endif
	char *buf;
	char *g;
	char *p, *wan;
	char *desc, *addr, *port, *prio, *transferred, *proto;
	int class_num;
	int down_class_num=6; 	// for download class_num = 0x6 / 0x106
	int i, inuse, unit;
	char q_inuse[32]; 	// for inuse
	char dport[256], saddr_1[192], proto_1[8], proto_2[8],conn[256], end[256], end2[256];
	char prefix[16];
	//int method;
	int gum;
	int sticky_enable;
	char chain[sizeof("QOSOXXX")];
	int v4v6_ok;
#ifdef CLS_ACT
	char imq_if[IFNAMSIZ];
#endif
	int lock;
	int evalRet;
	char *action = NULL;
	int model = get_model();

	switch (model) {
		case MODEL_DSLAX82U:
			action = "--set-mark";
			manual_return = 1;
			break;
		default:
			action = "--set-return";
			manual_return = 0;
			break;
	}

	del_iQosRules(); // flush related rules in mangle table

	if((fn = fopen(mangle_fn, "w")) == NULL) return -2;
#ifdef RTCONFIG_IPV6
	if(ipv6_enabled() && (fn_ipv6 = fopen(mangle_fn_ipv6, "w")) == NULL){
		fclose(fn);
		return -3;
	}
#endif

	lock = file_lock(qos_ipt_lock);
	QOSDBG("[qos] iptables START\n");
	fprintf(fn,
		"*mangle\n"
		":PREROUTING ACCEPT [0:0]\n"
		":OUTPUT ACCEPT [0:0]\n"
		);
#ifdef RTCONFIG_IPV6
	if (fn_ipv6 && ipv6_enabled())
	fprintf(fn_ipv6,
		"*mangle\n"
		":PREROUTING ACCEPT [0:0]\n"
		":OUTPUT ACCEPT [0:0]\n"
		);
#endif

	for (unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
		wan = get_wan_ifname(unit);
		if (!wan || *wan == '\0')
			continue;
		if (wan_primary_ifunit() != unit
#if defined(RTCONFIG_DUALWAN)
		    && (nvram_match("wans_mode", "fo") || nvram_match("wans_mode", "fb"))
#endif
		   )
			continue;
		fprintf(fn, ":QOSO%d - [0:0]\n"
			    "-A QOSO%d -j CONNMARK --restore-mark --mask 0x%x\n", unit, unit, QOS_MASK);
		fprintf(fn, "-A QOSO%d -m connmark ! --mark 0/0xff00 -j RETURN\n", unit);
#ifdef RTCONFIG_IPV6
		if (fn_ipv6 && ipv6_enabled())
		fprintf(fn_ipv6, ":QOSO%d - [0:0]\n" , unit);

		if (fn_ipv6 && ipv6_enabled() && wan_primary_ifunit() == unit) {
			fprintf(fn_ipv6, "-A QOSO%d -j CONNMARK --restore-mark --mask 0x%x\n", unit, QOS_MASK);
			fprintf(fn_ipv6, "-A QOSO%d -m connmark ! --mark 0/0xff00 -j RETURN\n", unit);
		}
#endif
	}

	for (unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
		wan = get_wan_ifname(unit);
		if (!wan || *wan == '\0')
			continue;
		if (wan_primary_ifunit() != unit
#if defined(RTCONFIG_DUALWAN)
		    && (nvram_match("wans_mode", "fo") || nvram_match("wans_mode", "fb"))
#endif
		   )
			continue;
#ifdef CLS_ACT
		snprintf(imq_if, sizeof(imq_if), "imq%d", unit);
		eval("ip", "link", "set", imq_if, "up");
#endif
		get_qos_prefix(unit, prefix);

		inuse = sticky_enable = 0;
		if (nvram_pf_match(prefix, "sticky", "0"))
			sticky_enable = 1;

		g = buf = strdup(nvram_pf_safe_get(prefix, "rulelist"));
		while (g) {

			/* ASUSWRT
			qos_rulelist :
				desc>addr>port>proto>transferred>prio

				addr  : (source) IP or MAC or IP-range
				port  : dest port
				proto : tcp, udp, tcp/udp, any , (icmp, igmp)
				transferred : min:max
				prio  : 0-4, 0 is the highest
			*/

			if ((p = strsep(&g, "<")) == NULL) break;
			if((vstrsep(p, ">", &desc, &addr, &port, &proto, &transferred, &prio)) != 6) continue;
			class_num = atoi(prio);
			if ((class_num < 0) || (class_num > 4)) continue;

			i = 1 << class_num;
			++class_num;

			if ((inuse & i) == 0) {
				inuse |= i;
				QOSDBG("[qos] iptable creates, inuse=%d\n", inuse);
			}

			v4v6_ok = IPT_V4;
#ifdef RTCONFIG_IPV6
			if (fn_ipv6 && ipv6_enabled())
				v4v6_ok |= IPT_V6;
#endif

			/* Beginning of the Rule */
			/*
				if transferred != NULL, class_num must bt 0x1~0x6, not 0x101~0x106
				0x1~0x6		: keep tracing this connection.
				0x101~0x106 	: connection will be considered as marked connection, won't detect again.
			*/
			gum = 0;
			class_num |= gum;
			down_class_num |= gum;	// for download

			snprintf(chain, sizeof(chain), "QOSO%d", unit);	// chain name
			snprintf(end , sizeof(end), " -j CONNMARK %s 0x%x/0x%x\n", action, class_num, QOS_MASK);	// CONNMARK string
			snprintf(end2, sizeof(end2), " -j RETURN\n");

			/*************************************************/
			/*                        addr                   */
			/*           src mac or src ip or IP range       */
			/*************************************************/
			char addr_new[40];
			int addr_type;
			memset(addr_new, 0, sizeof(addr_new));

			if(strcmp(addr, "")) {
				/* if addr != "", it needs to check the addr_type */
				address_format_checker(&addr_type, addr, addr_new, sizeof(addr_new));

				if (addr_type == TYPE_IP) {
					snprintf(saddr_1, sizeof(saddr_1), "-s %s", addr_new);
				}
				else if (addr_type == TYPE_MAC) {
					snprintf(saddr_1, sizeof(saddr_1), "-m mac --mac-source %s", addr_new);
				}
				else if (addr_type == TYPE_IPRANGE) {
					snprintf(saddr_1, sizeof(saddr_1), "-m iprange --src-range %s", addr_new);
				}
				else if (addr_type == TYPE_UNKNOWN) {
					QOSDBG("[qos] addr is TYPE_UKNOWN!\n");
					continue;
				}
				QOSLOG("[qos] addr_type=%d, saddr_1=%s", addr_type, saddr_1);
			}
			else {
				strncpy(saddr_1, "", sizeof(saddr_1));
			}

			/*************************************************/
			/*                      port                     */
			/*            single port or multi-ports         */
			/*************************************************/
			if(strcmp(port, "") == 0 ) {
				strncpy(dport, "", sizeof(dport));
			}
			else{
				/* note : max number of multiple port in iptables is 15 */
				snprintf(dport, sizeof(dport), "-m multiport --dport %s", port);
			}
			QOSLOG("[qos] port=%s, dport=%s", port, dport);

			/*************************************************/
			/*                   transferred                 */
			/*   --connbytes min:max                         */
			/*   --connbytes-dir (original/reply/both)       */
			/*   --connbytes-mode (packets/bytes/avgpkt)     */
			/*************************************************/
			char tmp[40];
			char *tmp_trans, *q_min, *q_max;
			long min = 0, max =0;

			sprintf(tmp, "%s", transferred);
			tmp_trans = tmp;
			q_min = strsep(&tmp_trans, "~");
			q_max = tmp_trans;

			if (strcmp(transferred,"") == 0){
				sprintf(conn, "%s", "");
			}
			else{
				sprintf(tmp, "%s", q_min);
				min = atol(tmp);

				if(strcmp(q_max,"") == 0) // q_max == NULL
					sprintf(conn, "-m connbytes --connbytes %ld:%s --connbytes-dir both --connbytes-mode bytes", min*1024, q_max);
				else{// q_max != NULL
					sprintf(tmp, "%s", q_max);
					max = atol(tmp);
					sprintf(conn, "-m connbytes --connbytes %ld:%ld --connbytes-dir both --connbytes-mode bytes", min*1024, max*1024-1);
				}
			}
			QOSLOG("[qos] tmp=%s, transferred=%s, min=%ld, max=%ld, q_max=%s, conn=%s", tmp, transferred, min*1024, max*1024-1, q_max, conn);

			/*************************************************/
			/*                      proto                    */
			/*        tcp, udp, tcp/udp, any, (icmp, igmp)   */
			/*************************************************/
			memset(proto_1, 0, sizeof(proto_1));
			memset(proto_2, 0, sizeof(proto_2));
			if(!strcmp(proto, "tcp"))
			{
				sprintf(proto_1, "-p tcp");
				sprintf(proto_2, "NO");
			}
			else if(!strcmp(proto, "udp"))
			{
				sprintf(proto_1, "-p udp");
				sprintf(proto_2, "NO");
			}
			else if(!strcmp(proto, "any"))
			{
				sprintf(proto_1, "%s", "");
				sprintf(proto_2, "NO");
			}
			else if(!strcmp(proto, "tcp/udp"))
			{
				sprintf(proto_1, "-p tcp");
				sprintf(proto_2, "-p udp");
			}
			else{
				sprintf(proto_1, "NO");
				sprintf(proto_2, "NO");
			}
			QOSLOG("[qos] proto_1=%s, proto_2=%s, proto=%s", proto_1, proto_2, proto);

			/*******************************************************************/
			/*                                                                 */
			/*  build final rule for check proto_1, proto_2, saddr_1           */
			/*                                                                 */
			/*******************************************************************/
			// step1. check proto != "NO"
			// step2. if proto = any, no proto / dport
			// step3. check saddr for ip-range; saddr_1 could be empty, dport only

			if (v4v6_ok & IPT_V4){
				// step1. check proto != "NO"
				if(strcmp(proto_1, "NO")){
					// step2. if proto = any, no proto / dport
					if(strcmp(proto_1, "")){
						// step3. check saddr for ip-range;saddr_1 could be empty, dport only
						fprintf(fn, "-A %s %s %s %s %s %s", chain, proto_1, dport, saddr_1, conn, end);
						if(manual_return)
						fprintf(fn, "-A %s %s %s %s %s %s", chain, proto_1, dport, saddr_1, conn, end2);
					}
					else{
						fprintf(fn, "-A %s %s %s %s", chain, saddr_1, conn, end);
						if(manual_return)
						fprintf(fn, "-A %s %s %s %s", chain, saddr_1, conn, end2);
					}
				}

				// step1. check proto != "NO"
				if(strcmp(proto_2, "NO")){
					// step2. if proto = any, no proto / dport
					if(strcmp(proto_2, "")){
						// step3. check saddr for ip-range;saddr_1 could be empty, dport only
						fprintf(fn, "-A %s %s %s %s %s %s", chain, proto_2, dport, saddr_1, conn, end);
						if(manual_return)
						fprintf(fn, "-A %s %s %s %s %s %s", chain, proto_2, dport, saddr_1, conn, end2);
					}
					else{
						fprintf(fn, "-A %s %s %s %s", chain, saddr_1, conn, end);
						if(manual_return)
						fprintf(fn, "-A %s %s %s %s", chain, saddr_1, conn, end2);
					}
				}
			}

#ifdef RTCONFIG_IPV6
			if (fn_ipv6 && ipv6_enabled() && (v4v6_ok & IPT_V6)){
				// step1. check proto != "NO"
				if(strcmp(proto_1, "NO")){
					// step2. if proto = any, no proto / dport
					if(strcmp(proto_1, "")){
						// step3. check saddr for ip-range;saddr_1 could be empty, dport only
						fprintf(fn_ipv6, "-A %s %s %s %s %s %s", chain, proto_1, dport, saddr_1, conn, end);
						if(manual_return)
						fprintf(fn_ipv6, "-A %s %s %s %s %s %s", chain, proto_1, dport, saddr_1, conn, end2);
					}
					else{
						fprintf(fn_ipv6, "-A %s %s %s %s", chain, saddr_1, conn, end);
						if(manual_return)
						fprintf(fn_ipv6, "-A %s %s %s %s", chain, saddr_1, conn, end2);
					}
				}

				// step1. check proto != "NO"
				if(strcmp(proto_2, "NO")){
					// step2. if proto = any, no proto / dport
					if(strcmp(proto_2, "")){
						// step3. check saddr for ip-range;saddr_1 could be empty, dport only
						fprintf(fn_ipv6, "-A %s %s %s %s %s %s", chain, proto_2, dport, saddr_1, conn, end);
						if(manual_return)
						fprintf(fn_ipv6, "-A %s %s %s %s %s %s", chain, proto_2, dport, saddr_1, conn, end2);
					}
					else{
						fprintf(fn_ipv6, "-A %s %s %s %s", chain, saddr_1, conn, end);
						if(manual_return)
						fprintf(fn_ipv6, "-A %s %s %s %s", chain, saddr_1, conn, end2);
					}
				}
			}
#endif
		}
		free(buf);

		/* lan_addr for iptables use (LAN download) */
		char *a, *b, *c, *d;
		char lan_addr[20];
		g = buf = strdup(nvram_safe_get("lan_ipaddr"));
		if((vstrsep(g, ".", &a, &b, &c, &d)) != 4){
			QOSDBG("[qos] lan_ipaddr doesn't exist!!\n");
		}
		else{
			sprintf(lan_addr, "%s.%s.%s.0/24", a, b, c);
			QOSDBG("[qos] lan_addr=%s\n", lan_addr);
		}
		free(buf);

		/* The default class */
		i = nvram_pf_get_int(prefix, "default");
		if ((i < 0) || (i > 4)) i = 3;  // "lowest"
		class_num = i + 1;

#ifdef CONFIG_BCMWL5 // TODO: it is only for the case, eth0 as wan, vlanx as lan
		if(strncmp(wan, "ppp", 3)==0){
			// ppp related interface doesn't need physdev
			// do nothing
		}
		else{
			/* for WLAN to LAN bridge packet */
			// ebtables : identify bridge packet
			add_EbtablesRules();

			// for multicast
			fprintf(fn, "-A %s -d 224.0.0.0/4 -j CONNMARK %s 0x%x/0x%x\n", chain, action, down_class_num, QOS_MASK);
			if(manual_return)
				fprintf(fn , "-A %s -d 224.0.0.0/4 -j RETURN\n", chain);
			// for download (LAN or wireless)
			fprintf(fn, "-A %s -d %s -j CONNMARK %s 0x%x/0x%x\n", chain, lan_addr, action, down_class_num, QOS_MASK);
			if(manual_return)
				fprintf(fn , "-A %s -d %s -j RETURN\n", chain, lan_addr);
	/* Requires bridge netfilter, but slows down and breaks EMF/IGS IGMP IPTV Snooping
			// for WLAN to LAN bridge issue
			fprintf(fn, "-A POSTROUTING -d %s -m physdev --physdev-is-in -j CONNMARK --set-return 0x6/0x%x\n", lan_addr, QOS_MASK);
	*/
			// for download, interface br0
			fprintf(fn, "-A POSTROUTING -o br0 -j %s\n", chain);
		}
#endif
			fprintf(fn, "-A %s -j CONNMARK %s 0x%x/0x%x\n", chain, action, class_num, QOS_MASK);
			if(manual_return)
				fprintf(fn , "-A %s -j RETURN\n", chain);
			fprintf(fn, "-A FORWARD -o %s -j %s\n", wan, chain);
			fprintf(fn, "-A OUTPUT -o %s -j %s\n", wan, chain);

#ifdef RTCONFIG_IPV6
		if (fn_ipv6 && ipv6_enabled() && *wan6face && wan_primary_ifunit() == unit) {
#ifdef CONFIG_BCMWL5 // TODO: it is only for the case, eth0 as wan, vlanx as lan
			if(strncmp(wan6face, "ppp", 3)==0){
				// ppp related interface doesn't need physdev
				// do nothing
			}
			else{
				/* for WLAN to LAN bridge packet */
				// ebtables : identify bridge packet
				add_EbtablesRules();

				// for multicast
				fprintf(fn_ipv6, "-A %s -d 224.0.0.0/4 -j CONNMARK %s 0x%x/0x%x\n", chain, action, down_class_num, QOS_MASK);
				if(manual_return)
					fprintf(fn_ipv6, "-A %s -d 224.0.0.0/4 -j RETURN\n", chain);
				// for download (LAN or wireless)
				fprintf(fn_ipv6, "-A %s -d %s -j CONNMARK %s 0x%x/0x%x\n", chain, lan_addr, action, down_class_num, QOS_MASK);
				if(manual_return)
					fprintf(fn_ipv6, "-A %s -d %s -j RETURN\n", chain, lan_addr);
	/* Requires bridge netfilter, but slows down and breaks EMF/IGS IGMP IPTV Snooping
				// for WLAN to LAN bridge issue
				fprintf(fn_ipv6, "-A POSTROUTING -d %s -m physdev --physdev-is-in -j CONNMARK --set-return 0x6/0x%x\n", lan_addr, QOS_MASK);
	*/
				// for download, interface br0
				fprintf(fn_ipv6, "-A POSTROUTING -o br0 -j %s\n", chain);
			}
#endif
			fprintf(fn_ipv6, "-A %s -j CONNMARK %s 0x%x/0x%x\n", chain, action, class_num, QOS_MASK);
			if(manual_return)
				fprintf(fn_ipv6, "-A %s -j RETURN\n", chain);
			fprintf(fn_ipv6, "-A FORWARD -o %s -j %s\n", wan6face, chain);
			fprintf(fn_ipv6, "-A OUTPUT -o %s -j %s\n", wan6face, chain);
		}
#endif

		inuse |= (1 << i) | 1;  // default and highest are always built
		sprintf(q_inuse, "%d", inuse);
		nvram_pf_set(prefix, "inuse", q_inuse);
		QOSDBG("[qos] qos_inuse=%d\n", inuse);

		/* Ingress rules */
		g = buf = strdup(nvram_pf_safe_get(prefix, "irates"));
		for (i = 0; i < 10; ++i) {
			if ((!g) || ((p = strsep(&g, ",")) == NULL)) continue;
			if ((inuse & (1 << i)) == 0) continue;
			if (atoi(p) > 0) {
				fprintf(fn, "-A PREROUTING -i %s -j CONNMARK --restore-mark --mask 0x%x\n", wan, QOS_MASK);
#ifdef CLS_ACT
				fprintf(fn, "-A PREROUTING -i %s -j IMQ --todev %d\n", wan, unit);
#endif
#ifdef RTCONFIG_IPV6
				if (fn_ipv6 && ipv6_enabled() && *wan6face && wan_primary_ifunit() == unit) {
					fprintf(fn_ipv6, "-A PREROUTING -i %s -j CONNMARK --restore-mark --mask 0x%x\n", wan6face, QOS_MASK);
#ifdef CLS_ACT
					fprintf(fn_ipv6, "-A PREROUTING -i %s -j IMQ --todev %d\n", wan6face, unit);
#endif
				}
#endif
				break;
			}
		}
		free(buf);
	} /* for (u = WAN_UNIT_FIRST; u < WAN_UNIT_MAX; ++u) */

	fprintf(fn, "COMMIT\n");
	fclose(fn);
	chmod(mangle_fn, 0700);
	evalRet = eval("iptables-restore", "-n", (char*)mangle_fn);
	rule_apply_checking("qos_multiwan", __LINE__, (char*)mangle_fn, evalRet);
#ifdef RTCONFIG_IPV6
	if (fn_ipv6 && ipv6_enabled())
	{
		fprintf(fn_ipv6, "COMMIT\n");
		fclose(fn_ipv6);
		chmod(mangle_fn_ipv6, 0700);

//		eval("ip6tables-restore", "-n", (char*)mangle_fn_ipv6);
//		ipv6_qos_applied++;
	}
#endif
	file_unlock(lock);
	QOSDBG("[qos] iptables DONE!\n");

	return 0;
}


/*******************************************************************/
// The definations of all partations
// eth0 : WAN
// 1:1  : upload
// 1:2  : download   (10240000Kbits = 10Gbps)
// 1:10 : highest
// 1:20 : high
// 1:30 : middle
// 1:40 : low        (default)
// 1:50 : lowest
// 1:60 : ALL Download (WAN to LAN and LAN to LAN) (10240000kbits = 10Gbps)
/*******************************************************************/

/* Tc */
static int start_tqos(void)
{
	static char wan_if_list[WAN_UNIT_MAX][IFNAMSIZ] = { "", "" };
	int i, ret = 0, gen_mangle = 0;
	char *buf, *g, *p;
	unsigned int rate;
	unsigned int ceil;
	unsigned int ibw, obw, bw;
	unsigned int mtu;
	FILE *f, *f_top;
	int x, lock;
	int inuse;
	char s[256], *wan;
	int first, unit;
	char fname[sizeof(TMP_QOS) + 4];	/* /tmp/qos, /tmp/qos.0, /tmp/qos.1 ... */
	char prefix[16], imq_if[IFNAMSIZ];
	char burst_root[32];
	char burst_leaf[32];
	char *qsched;
#ifdef CONFIG_BCMWL5
	char *protocol="802.1q";
#endif

	/* If WAN interface changes, generate mangle table again. */
	for (unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
		if (wan_primary_ifunit() != unit
#if defined(RTCONFIG_DUALWAN)
		    && (nvram_match("wans_mode", "fo") || nvram_match("wans_mode", "fb"))
#endif
		   )
			continue;
		wan = get_wan_ifname(unit);
		if (!wan || *wan == '\0' || !strcmp(wan, wan_if_list[unit]))
			continue;

		sprintf(prefix, "wan%d_", unit);
		if (!nvram_pf_match(prefix, "state_t", "2"))
			continue;
		gen_mangle++;
		strcpy(wan_if_list[unit], wan);
	}
	if (gen_mangle)
		add_qos_rules("");

	lock = file_lock(qos_lock);

	if (!(f_top = fopen(qosfn, "w"))) {
		ret = -2;
		goto exit_start_tqos;
	}

	/* Create top-level /tmp/qos */
	QOSDBG("[qos] tc START!\n");
	fprintf(f_top,"#!/bin/sh\n");

	/* Create /tmp/qos.X for each WAN unit. */
	for (unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
		snprintf(fname, sizeof(fname), "%s.%d", TMP_QOS, unit);
		wan = get_wan_ifname(unit);
		if (!wan || *wan == '\0')
			continue;
		snprintf(imq_if, sizeof(imq_if), "imq%d", unit);
		if (wan_primary_ifunit() != unit
#if defined(RTCONFIG_DUALWAN)
		    && (nvram_match("wans_mode", "fo") || nvram_match("wans_mode", "fb"))
#endif
		   )
		{
			if (f_exists(fname))
				unlink(fname);
			/* Remove qdisc of unused WAN. Ref to stop case below. */
			eval("tc", "qdisc", "del", "dev", wan, "root");
#ifdef CLS_ACT
			eval("tc", "qdisc", "del", "dev", imq_if, "root");
#else
			eval("tc", "qdisc", "del", "dev", wan, "ingress");
#endif
			continue;
		}
		if (!(f = fopen(fname, "w"))) {
			fprintf(stderr, "[qos] Can't write to %s\n", fname);
			continue;
		}
		get_qos_prefix(unit, prefix);

		// judge interface by get_wan_ifname
		// add Qos iptable rules in mangle table,
		// move it to firewall - mangle_setting
		// add_iQosRules(get_wan_ifname(unit)); // iptables start

#if defined(RTCONFIG_DUALWAN)
		if(unit != 0)
		{
			char ibw_str[8], obw_str[8];

			snprintf(ibw_str, sizeof(ibw_str), "ibw%d", unit);
			snprintf(obw_str, sizeof(obw_str), "obw%d", unit);
			ibw = strtoul(nvram_pf_safe_get(prefix, ibw_str), NULL, 10);
			obw = strtoul(nvram_pf_safe_get(prefix, obw_str), NULL, 10);
		}
		else
#endif
		{
			ibw = strtoul(nvram_pf_safe_get(prefix, "ibw"), NULL, 10);
			obw = strtoul(nvram_pf_safe_get(prefix, "obw"), NULL, 10);
		}
		if (!ibw || !obw)
			continue;

		fprintf(stderr, "[qos][unit %d] tc START!\n", unit);

		/* qos_burst */
		i = nvram_pf_get_int(prefix, "burst0");
		if(i > 0) sprintf(burst_root, "burst %dk", i);
			else burst_root[0] = 0;
		i = nvram_pf_get_int(prefix, "burst1");

		if(i > 0) sprintf(burst_leaf, "burst %dk", i);
			else burst_leaf[0] = 0;

		/* Egress OBW  -- set the HTB shaper (Classful Qdisc)
		 * the BW is set here for each class
		 */

		/* FIXME: unused nvram variable? */
		mtu = strtoul(nvram_safe_get("wan_mtu"), NULL, 10);
		bw = obw;

#if defined(RTCONFIG_HND_ROUTER_AX)
		qsched = "fq_codel quantum 300 limit 1000 noecn";
#else
		qsched = "sfq perturb 10";
#endif

		/* WAN */
		fprintf(f,
			"#!/bin/sh\n"
			"#LAN/WAN\n"
			"SCH=\"%s\"\n"
			"I=%s\n"
			"TQA=\"tc qdisc add dev $I\"\n"
			"TCA=\"tc class add dev $I\"\n"
			"TFA=\"tc filter add dev $I\"\n"
#ifdef CLS_ACT
			"DLIF=%s\n"
			"TQADL=\"tc qdisc add dev $DLIF\"\n"
			"TCADL=\"tc class add dev $DLIF\"\n"
			"TFADL=\"tc filter add dev $DLIF\"\n"
#endif
			"case \"$1\" in\n"
			"start)\n"
			"#LAN/WAN\n"
			"\ttc qdisc del dev $I root 2>/dev/null\n"
			"\t$TQA root handle 1: htb default %u\n"
#ifdef CLS_ACT
			"\ttc qdisc del dev $DLIF root 2>/dev/null\n"
			"\t$TQADL root handle 2: htb default %u\n"
#endif
			"# upload 2:1\n"
			"\t$TCA parent 1: classid 1:1 htb rate %ukbit ceil %ukbit %s\n" ,
				qsched,
				wan, imq_if,
				(nvram_pf_get_int(prefix, "default") + 1) * 10,
#ifdef CLS_ACT
				(nvram_pf_get_int(prefix, "default") + 1) * 10,
#endif
				bw, bw, burst_root);

		/* LAN protocol: 802.1q */
#ifdef CONFIG_BCMWL5 // TODO: it is only for the case, eth0 as wan, vlanx as lan
		if (!unit) {
			protocol = "802.1q";
			fprintf(f,
				"# download 1:2\n"
				"\t$TCA parent 1: classid 1:2 htb rate 10240000kbit ceil 10240000kbit burst 10000 cburst 10000\n"
				"# 1:60 ALL Download for BCM\n"
				"\t$TCA parent 1:2 classid 1:60 htb rate 10240000kbit ceil 10240000kbit burst 10000 cburst 10000 prio 6\n"
				"\t$TQA parent 1:60 handle 60: pfifo\n"
				"\t$TFA parent 1: prio 6 protocol %s u32 match mark 6 0x%x flowid 1:60\n", protocol, QOS_MASK
				);
		}
#endif

		inuse = nvram_pf_get_int(prefix, "inuse");

		g = buf = strdup(nvram_pf_safe_get(prefix, "orates"));
		for (i = 0; i < 5; ++i) { // 0~4 , 0:highest, 4:lowest

			if ((!g) || ((p = strsep(&g, ",")) == NULL)) break;

			if ((inuse & (1 << i)) == 0){
				QOSDBG("[qos] egress %d doesn't create, inuse=%d\n", i, inuse);
				continue;
			}
			else {
				QOSDBG("[qos] egress %d creates\n", i);
			}

			if ((sscanf(p, "%u-%u", &rate, &ceil) != 2) || (rate < 1)) {
				continue;
			}

			if (ceil > 0) sprintf(s, "ceil %ukbit ", calc(bw, ceil));
				else s[0] = 0;
			x = (i + 1) * 10;

			fprintf(f, "# egress %d: %u-%u%%\n", i, rate, ceil);
			fprintf(f, "\t$TCA parent 1:1 classid 1:%d htb rate %ukbit %s %s prio %d quantum %u\n", x, calc(bw, rate), s, burst_leaf, (i >= 6) ? 7 : (i + 1), mtu);
			fprintf(f, "\t$TQA parent 1:%d handle %d: $SCH\n", x, x);
			fprintf(f, "\t$TFA parent 1: prio %d protocol ip u32 match mark %d 0x%x flowid 1:%d\n", x, i + 1, QOS_MASK, x);
		}
		free(buf);

		/*
			10000 = ACK
			00100 = RST
			00010 = SYN
			00001 = FIN
		*/

		if (nvram_pf_match(prefix, "ack", "on")) {
			fprintf(f,
				"\n"
				"\t$TFA parent 1: prio 14 protocol ip u32 "
				"match ip protocol 6 0xff "			// TCP
				"match u8 0x05 0x0f at 0 "			// IP header length
				"match u16 0x0000 0xffc0 at 2 "			// total length (0-63)
				"match u8 0x10 0xff at 33 "			// ACK only
				"flowid 1:10\n");
		}
		if (nvram_pf_match(prefix, "syn", "on")) {
			fprintf(f,
				"\n"
				"\t$TFA parent 1: prio 15 protocol ip u32 "
				"match ip protocol 6 0xff "			// TCP
				"match u8 0x05 0x0f at 0 "			// IP header length
				"match u16 0x0000 0xffc0 at 2 "			// total length (0-63)
				"match u8 0x02 0x02 at 33 "			// SYN,*
				"flowid 1:10\n");
		}
		if (nvram_pf_match(prefix, "fin", "on")) {
			fprintf(f,
				"\n"
				"\t$TFA parent 1: prio 17 protocol ip u32 "
				"match ip protocol 6 0xff "			// TCP
				"match u8 0x05 0x0f at 0 "			// IP header length
				"match u16 0x0000 0xffc0 at 2 "			// total length (0-63)
				"match u8 0x01 0x01 at 33 "			// FIN,*
				"flowid 1:10\n");
		}
		if (nvram_pf_match(prefix, "rst", "on")) {
			fprintf(f,
				"\n"
				"\t$TFA parent 1: prio 19 protocol ip u32 "
				"match ip protocol 6 0xff "			// TCP
				"match u8 0x05 0x0f at 0 "			// IP header length
				"match u16 0x0000 0xffc0 at 2 "			// total length (0-63)
				"match u8 0x04 0x04 at 33 "			// RST,*
				"flowid 1:10\n");
		}
		if (nvram_pf_match(prefix, "icmp", "on")) {
			fprintf(f, "\n\t$TFA parent 1: prio 13 protocol ip u32 match ip protocol 1 0xff flowid 1:10\n\n");
		}

		// ingress
		first = 1;
		bw = ibw;

		if (bw > 0) {
			g = buf = strdup(nvram_pf_safe_get(prefix, "irates"));
			for (i = 0; i < 5; ++i) { // 0~4 , 0:highest, 4:lowest
				if ((!g) || ((p = strsep(&g, ",")) == NULL)) break;
				if ((inuse & (1 << i)) == 0) continue;
				if ((rate = atoi(p)) < 1) continue;	// 0 = off

				if (first) {
					first = 0;
					fprintf(f,
						"\n"
#if !defined(CLS_ACT)
						"\ttc qdisc del dev $I ingress 2>/dev/null\n"
						"\t$TQA handle ffff: ingress\n"
#endif
						);
				}

				// rate in kb/s
				unsigned int u = calc(bw, rate);

				// burst rate
				unsigned int v = u / 2;
				if (v < 50) v = 50;

#ifdef CLS_ACT
				x = (i + 1) * 10;
				fprintf(f, "# ingress %d: %u%%\n", i, rate);
				fprintf(f,"\t$TCADL parent 2:1 classid 2:%d htb rate %ukbit %s prio %d quantum %u\n", x, calc(bw, rate), burst_leaf, (i >= 6) ? 7 : (i + 1), mtu);
				fprintf(f,"\t$TQADL parent 2:%d handle %d: $SCH\n", x, x);
				fprintf(f,"\t$TFADL parent 2: prio %d protocol ip u32 match mark %d 0x%x flowid 2:%d\n", x, i + 1, QOS_MASK, x);
#else
				x = i + 1;
				fprintf(f,
					"# ingress %d: %u%%\n", i, rate);
				fprintf(f,"\t$TFA parent ffff: prio %d protocol ip handle %d "
					  "fw police rate %ukbit burst %ukbit drop flowid ffff:%d\n", x, x, u, v, x);
#endif
			}
			free(buf);
		}

		fputs(
			"\t;;\n"
			"stop)\n"
			"\ttc qdisc del dev $I root 2>/dev/null\n"
#ifdef CLS_ACT
			"\ttc qdisc del dev $DLIF root 2>/dev/null\n"
#else
			"\ttc qdisc del dev $I ingress 2>/dev/null\n"
#endif
			"\t;;\n"
			"*)\n"
			"\t#---------- Upload ----------\n"
			"\ttc -s -d class ls dev $I\n"
			"\ttc -s -d qdisc ls dev $I\n"
			"\techo\n"
#ifdef CLS_ACT
			"\t#--------- Download ---------\n"
			"\ttc -s -d class ls dev $DLIF\n"
			"\ttc -s -d qdisc ls dev $DLIF\n"
			"\techo\n"
#endif
			"esac\n",
			f);

		fclose(f);
		chmod(fname, 0700);

		fprintf(f_top, "[ -e %s ] && %s \"$1\"\n", fname, fname);
		QOSDBG("[qos]unit %d] tc done!\n", unit);
	} /* for (unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) */

	fclose(f_top);
	chmod(qosfn, 0700);
	eval((char *)qosfn, "start");
	QOSDBG("[qos] tc done!\n");

exit_start_tqos:
	file_unlock(lock);

	return ret;
}


void stop_iQos(void)
{
	eval((char *)qosfn, "stop");

#ifdef HND_ROUTER
	config_obw_off();
#endif
}

static int add_bandwidth_limiter_rules(char *pcWANIF)
{
	FILE *fn = NULL;
	char *buf, *g, *p;
	char *enable, *addr, *dlc, *upc, *prio;
	char lan_addr[32];
	char addr_new[40];
	char prefix[32];
	int addr_type;
	char *action = NULL, *wan;
	int r, lock, unit;
	int class = 0;
	char imq_if[IFNAMSIZ];
	char act_buf[sizeof("CONNMARK --set-return X/0xXXXXXXX")], *acts[2] = { act_buf , "RETURN" };
	int evalRet;

	del_iQosRules(); // flush related rules in mangle table
	if ((fn = fopen(mangle_fn, "w")) == NULL) return -2;

	lock = file_lock(qos_ipt_lock);
	switch (get_model()){
		case MODEL_DSLN55U:
		case MODEL_RTN13U:
		case MODEL_RTN56U:
			action = "CONNMARK --set-return";
			manual_return = 1;
			break;
		default:
			action = "MARK --set-mark";
			manual_return = 2;
			break;
	}

	/* ASUSWRT
	qos_bw_rulelist :
		enable>addr>DL-Ceil>UL-Ceil>prio
		enable : enable or disable this rule
		addr : (source) IP or MAC or IP-range or wireless interface(wl0.1, wl0.2, etc.)
		DL-Ceil : the max download bandwidth
		UL-Ceil : the max upload bandwidth
		prio : priority for client
	*/

	memset(lan_addr, 0, sizeof(lan_addr));
	sprintf(lan_addr, "%s/%s", nvram_safe_get("lan_ipaddr"), nvram_safe_get("lan_netmask"));

	fprintf(fn,
		"*mangle\n"
		":PREROUTING ACCEPT [0:0]\n"
		":OUTPUT ACCEPT [0:0]\n"
		);

	// access router : mark 9
	snprintf(act_buf, sizeof(act_buf), "%s 9/0x%x", action, QOS_MASK);
	for (r = 0; r < manual_return; ++r) {
		fprintf(fn, "-A POSTROUTING -s %s -d %s -j %s\n", nvram_safe_get("lan_ipaddr"), lan_addr, acts[r]);
		fprintf(fn, "-A PREROUTING -s %s -d %s -j %s\n", lan_addr, nvram_safe_get("lan_ipaddr"), acts[r]);
	}

	get_qos_prefix(0, prefix);

	g = buf = strdup(nvram_pf_safe_get(prefix, "bw_rulelist"));
	while (g) {
		if ((p = strsep(&g, "<")) == NULL) break;
		if ((vstrsep(p, ">", &enable, &addr, &dlc, &upc, &prio)) != 5) continue;
		if (!strcmp(enable, "0")) continue;

		memset(addr_new, 0, sizeof(addr_new));
		address_format_checker(&addr_type, addr, addr_new, sizeof(addr_new));
		class = atoi(prio) + INITIAL_MARKNUM;
		QOSDBG("[BWLIT] addr_type=%d, addr=%s, add_new=%s, lan_addr=%s\n", addr_type, addr, addr_new, lan_addr);

#ifdef RTCONFIG_PERMISSION_MANAGEMENT
		if (*addr == '@') {
			int dev_num, group_num;
			PMS_DEVICE_INFO_T *dev_list = NULL;
			PMS_DEVICE_GROUP_INFO_T *group_list = NULL, *follow_group = NULL;

			/* Get account / group list */
			if (PMS_GetDeviceInfo(PMS_ACTION_GET_FULL, &dev_list, &group_list, &dev_num, &group_num) < 0) {
				_dprintf("Can't read dev / group list\n");
				break;
			}

			/* Get the mac list of certain group */
			for (follow_group = group_list; follow_group != NULL; follow_group = follow_group->next) {
				PMS_OWNED_INFO_T *owned_dev;

				if (strcmp(follow_group->name, addr+1))
					continue;

				owned_dev = follow_group->owned_device;
				while (owned_dev != NULL) {
					PMS_DEVICE_INFO_T *dev_owned = (PMS_DEVICE_INFO_T *) owned_dev->member;
					//_dprintf("[%s] %s\n", follow_group->name, dev_owned->mac); // debug
					snprintf(act_buf, sizeof(act_buf), "%s %d/0x%x", action, class, QOS_MASK);
					for (r = 0; r < manual_return; ++r) {
						fprintf(fn, "-A PREROUTING -m mac --mac-source %s ! -d %s -j %s\n", dev_owned->mac, lan_addr, acts[r]);
					}
					owned_dev = owned_dev->next;
				}
			}

			/* Free device and group list*/
			PMS_FreeDevInfo(&dev_list, &group_list);
		}
#endif

		if (addr_type == TYPE_IP){
			snprintf(act_buf, sizeof(act_buf), "%s %d/0x%x", action, class, QOS_MASK);
			for (r = 0; r < manual_return; ++r) {
				fprintf(fn, "-A POSTROUTING ! -s %s -d %s -j %s\n", lan_addr, addr_new, acts[r]);
				fprintf(fn, "-A PREROUTING -s %s ! -d %s -j %s\n", addr_new, lan_addr, acts[r]);
			}
		}
		else if (addr_type == TYPE_MAC){
			snprintf(act_buf, sizeof(act_buf), "%s %d/0x%x", action, class, QOS_MASK);
			for (r = 0; r < manual_return; ++r) {
				fprintf(fn, "-A PREROUTING -m mac --mac-source %s ! -d %s -j %s\n" , addr_new, lan_addr, acts[r]);
			}
		}
		else if (addr_type == TYPE_IPRANGE){
			snprintf(act_buf, sizeof(act_buf), "%s %d/0x%x", action, class, QOS_MASK);
			for (r = 0; r < manual_return; ++r) {
				fprintf(fn, "-A POSTROUTING ! -s %s -m iprange --dst-range %s -j %s\n", lan_addr, addr_new, acts[r]);
				fprintf(fn, "-A PREROUTING -m iprange --src-range %s ! -d %s -j %s\n", addr_new, lan_addr, acts[r]);
			}
		}
		else if (addr_type == TYPE_GROUP) {
			continue;
		}
	}
	free(buf);

#if defined(RTCONFIG_DUALWAN)
	if (nvram_match("wans_mode", "lb")) {
		snprintf(imq_if, sizeof(imq_if), "imq%d", BWLIT_IMQ_ID);
		eval("ip", "link", "set", imq_if, "up");
		for (unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
			wan = get_wan_ifname(unit);
			if (!wan || *wan == '\0')
				continue;

			sprintf(prefix, "wan%d_", unit);
			if (!nvram_pf_match(prefix, "state_t", "2"))
				continue;
			fprintf(fn, "-A POSTROUTING -o %s -j IMQ --todev %d\n", wan, BWLIT_IMQ_ID);
		}
	}
#endif

#ifdef RTCONFIG_AMAS_WGN
	// AMAS RE mode
	if (nvram_get_int("re_mode") == 1) add_iptables_AMAS_WGN(fn, action);
#endif

	fprintf(fn, "COMMIT\n");
	fclose(fn);
	chmod(mangle_fn, 0700);
	evalRet = eval("iptables-restore", "-n", (char*)mangle_fn);
	rule_apply_checking("qos_multiwan", __LINE__, (char*)mangle_fn, evalRet);
	QOSDBG("[BWLIT] Create iptables rules done.\n");
	
	/* Setup guest network's ebtables rules */
	add_EbtablesRules_BW();
	file_unlock(lock);

	return 0;
}

static int guest; // qdisc root only 3: ~ 14: (12 guestnetwork)

static int start_bandwidth_limiter(void)
{
	static char wan_if_list[WAN_UNIT_MAX][IFNAMSIZ] = { "", "" };
	FILE *f;
	char *buf, *g, *p, *wan, ul_ifname[IFNAMSIZ];
	char *enable, *addr, *dlc, *upc, *prio;
	char prefix[16];
	int class = 0, unit, ret = 0, gen_mangle = 0;
	int s[6]; // strip mac address
	int addr_type, lock;		/* handler of br0 for each WAN unit */
	char addr_new[40];
	char wl_ifname[IFNAMSIZ];
	char *qsched;

	/* If WAN interface changes, generate mangle table again. */
	for (unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
		if (wan_primary_ifunit() != unit
#if defined(RTCONFIG_DUALWAN)
		    && (nvram_match("wans_mode", "fo") || nvram_match("wans_mode", "fb"))
#endif
		   )
			continue;
		wan = get_wan_ifname(unit);
		if (!wan || *wan == '\0' || !strcmp(wan, wan_if_list[unit]))
			continue;

		sprintf(prefix, "wan%d_", unit);
		if (!nvram_pf_match(prefix, "state_t", "2"))
			continue;
		gen_mangle++;
		strcpy(wan_if_list[unit], wan);
	}
	if (gen_mangle)
		add_bandwidth_limiter_rules("");

	lock = file_lock(qos_lock);

	if (!(f = fopen(qosfn, "w"))) {
		ret = -1;
		goto exit_start_bandwidth_limiter;
	}

	strlcpy(ul_ifname, get_wan_ifname(wan_primary_ifunit()), sizeof(ul_ifname));
#if defined(RTCONFIG_DUALWAN)
	if (nvram_match("wans_mode", "lb")) {
		snprintf(ul_ifname, sizeof(ul_ifname), "imq%d", BWLIT_IMQ_ID);
	}
#endif
	wan = ul_ifname;

	/* Remove qdisc of br0/WAN. Ref to stop case below. */
	fprintf(f, "#!/bin/sh\n"
		   "tc qdisc del dev br0 root 2>/dev/null\n"
		   "tc qdisc del dev br0 ingress 2>/dev/null\n");
	fprintf(f, "tc qdisc del dev %s root 2>/dev/null\n", wan);
	fprintf(f, "tc qdisc del dev %s ingress 2>/dev/null\n", wan);

	get_qos_prefix(0, prefix);
	guest = 3;	/* 3 ~ 12 ==> egress from guest network, handle (qdisc-id) */

#if defined(RTCONFIG_HND_ROUTER_AX)
	qsched = "fq_codel quantum 300 limit 1000 noecn";
#else
	qsched = "sfq perturb 10";
#endif

	fprintf(f, "#!/bin/sh\n"
		   "SCH=\"%s\"\n"
		   "WAN=%s\n"
		   , qsched
		   , wan);
	fprintf(f, "tc qdisc del dev $WAN root 2>/dev/null\n"
		   "tc qdisc del dev $WAN ingress 2>/dev/null\n"
		   "\n"
		   "TQAU=\"tc qdisc add dev $WAN\"\n"
		   "TCAU=\"tc class add dev $WAN\"\n"
		   "TFAU=\"tc filter add dev $WAN\"\n"
		   "TQA=\"tc qdisc add dev br0\"\n"
		   "TCA=\"tc class add dev br0\"\n"
		   "TFA=\"tc filter add dev br0\"\n"
		   "\n"
		   "start()\n"
		   "{\n"
		   "\t$TQA root handle 1: htb\n"
		   "\t$TCA parent 1: classid 1:1 htb rate 10240000kbit\n"
		   "\n"
		   "\t$TQAU root handle 2: htb\n"
		   "\t$TCAU parent 2: classid 2:1 htb rate 10240000kbit\n"
	);

	// access router : mark 9
	// default : 10Gbps
	fprintf(f, "\n"
		   "\t$TCA parent 1:1 classid 1:9 htb rate %ukbit ceil %ukbit prio 1\n", max_wire_speed, max_wire_speed);
	fprintf(f, "\t$TQA parent 1:9 handle 9: $SCH\n");
	fprintf(f, "\t$TFA parent 1: prio 1 protocol ip u32 match mark 9 0x%x flowid 1:9\n", QOS_MASK);
	fprintf(f, "\n"
		   "\t$TCAU parent 2:1 classid 2:9 htb rate %ukbit ceil %ukbit prio 1\n", max_wire_speed, max_wire_speed);
	fprintf(f, "\t$TQAU parent 2:9 handle 9: $SCH\n");
	fprintf(f, "\t$TFAU parent 2: prio 1 protocol ip u32 match mark 9 0x%x flowid 2:9\n", QOS_MASK);

	/* ASUSWRT
	qos_bw_rulelist :
		enable>addr>DL-Ceil>UL-Ceil>prio
		enable : enable or disable this rule
		addr : (source) IP or MAC or IP-range
		DL-Ceil : the max download bandwidth
		UL-Ceil : the max upload bandwidth
		prio : priority for client
	*/

	g = buf = strdup(nvram_safe_get("qos_bw_rulelist"));

	while (g) {
		if ((p = strsep(&g, "<")) == NULL) break;
		if ((vstrsep(p, ">", &enable, &addr, &dlc, &upc, &prio)) != 5) continue;
		if (!strcmp(enable, "0")) continue;

		address_format_checker(&addr_type, addr, addr_new, sizeof(addr_new));
		class = atoi(prio) + INITIAL_MARKNUM;

#ifdef RTCONFIG_PERMISSION_MANAGEMENT
		if (*addr == '@') {
			int dev_num, group_num;
			PMS_DEVICE_INFO_T *dev_list = NULL;
			PMS_DEVICE_GROUP_INFO_T *group_list = NULL, *follow_group = NULL;

			/* Get account / group list */
			if (PMS_GetDeviceInfo(PMS_ACTION_GET_FULL, &dev_list, &group_list, &dev_num, &group_num) < 0) {
				_dprintf("Can't read dev / group list\n");
				break;
			}

			/* Get the mac list of certain group */
			for (follow_group = group_list; follow_group != NULL; follow_group = follow_group->next) {
				PMS_OWNED_INFO_T *owned_dev;

				if (strcmp(follow_group->name, addr+1))
					continue;

				owned_dev = follow_group->owned_device;
				while (owned_dev != NULL) {
					PMS_DEVICE_INFO_T *dev_owned = (PMS_DEVICE_INFO_T *) owned_dev->member;
					//_dprintf("[%s] %s\n", follow_group->name, dev_owned->mac); // debug
					sscanf(dev_owned->mac, "%02X:%02X:%02X:%02X:%02X:%02X",&s[0],&s[1],&s[2],&s[3],&s[4],&s[5]);
					fprintf(f, "\n"
						   "$TCA parent 1:1 classid 1:%d htb rate %skbit ceil %skbit prio %d\n", class, dlc, dlc, class);
					fprintf(f, "$TQA parent 1:%d handle %d: $SCH\n", class, class);
					fprintf(f, "$TFA parent 1: protocol ip prio %d u32 match u16 0x0800 0xFFFF at -2"
						   " match u32 0x%02X%02X%02X%02X 0xFFFFFFFF at -12"
						   " match u16 0x%02X%02X 0xFFFF at -14 flowid 1:%d",
						   class, s[2], s[3], s[4], s[5], s[0], s[1], class);
					fprintf(f, "\n");
					fprintf(f, "$TCAU parent 2:1 classid 2:%d htb rate %skbit ceil %skbit prio %d\n", class, upc, upc, class);
					fprintf(f, "$TQAU parent 2:%d handle %d: $SCH\n", class, class);
					fprintf(f, "$TFAU parent 2: prio %d protocol ip u32 match mark %d 0x%x flowid 2:%d\n", class, class, QOS_MASK, class);
					owned_dev = owned_dev->next;
				}
			}

			/* Free device and group list*/
			PMS_FreeDevInfo(&dev_list, &group_list);
		}
#endif

		if (addr_type == TYPE_MAC)
		{
			sscanf(addr_new, "%02X:%02X:%02X:%02X:%02X:%02X",&s[0],&s[1],&s[2],&s[3],&s[4],&s[5]);
			fprintf(f, "\n"
				   "\t$TCA parent 1:1 classid 1:%d htb rate %skbit ceil %skbit prio %d\n", class, dlc, dlc, class);
			fprintf(f, "\t$TQA parent 1:%d handle %d: $SCH\n", class, class);
			fprintf(f, "\t$TFA parent 1: protocol ip prio %d u32 match u16 0x0800 0xFFFF at -2"
				   " match u32 0x%02X%02X%02X%02X 0xFFFFFFFF at -12"
				   " match u16 0x%02X%02X 0xFFFF at -14 flowid 1:%d",
				   class, s[2], s[3], s[4], s[5], s[0], s[1], class);
			fprintf(f, "\n"
				   "\t$TCAU parent 2:1 classid 2:%d htb rate %skbit ceil %skbit prio %d\n", class, upc, upc, class);
			fprintf(f, "\t$TQAU parent 2:%d handle %d: $SCH\n", class, class);
			fprintf(f, "\t$TFAU parent 2: prio %d protocol ip u32 match mark %d 0x%x flowid 2:%d\n", class, class, QOS_MASK, class);
		}
		else if (addr_type == TYPE_IP || addr_type == TYPE_IPRANGE)
		{
			fprintf(f, "\n"
				   "\t$TCA parent 1:1 classid 1:%d htb rate %skbit ceil %skbit prio %d\n", class, dlc, dlc, class);
			fprintf(f, "\t$TQA parent 1:%d handle %d: $SCH\n", class, class);
			fprintf(f, "\t$TFA parent 1: prio %d protocol ip u32 match mark %d 0x%x flowid 1:%d\n", class, class, QOS_MASK, class);
			fprintf(f, "\n"
				   "\t$TCAU parent 2:1 classid 2:%d htb rate %skbit ceil %skbit prio %d\n", class, upc, upc, class);
			fprintf(f, "\t$TQAU parent 2:%d handle %d: $SCH\n", class, class);
			fprintf(f, "\t$TFAU parent 2: prio %d protocol ip u32 match mark %d 0x%x flowid 2:%d\n", class, class, QOS_MASK, class);
		}
	}

	// init guest 3: ~ 14: (12 guestnetwork), start number = 3
	guest = 3;
	int  guest_mark = GUEST_INIT_MARKNUM;
	char wl[128], wlv[128], tmp[128], *next, *next2;
	char *wl_if = wl_ifname;
	int  i = 0;
	int  j = 1;

	/* Setup guest network's bandwidth limiter */
	foreach(wl, nvram_safe_get("wl_ifnames"), next) {
		SKIP_ABSENT_BAND_AND_INC_UNIT(i);
		snprintf(prefix, sizeof(prefix), "wl%d_", i);
		foreach(wlv, nvram_safe_get(strcat_r(prefix, "vifnames", tmp)), next2) {
			if (!nvram_pf_get_int(wlv, "_bss_enabled") ||
			    !nvram_pf_get_int(wlv, "_bw_enabled")) {
				j++;
				continue;
			}
			get_wlxy_ifname(i, j, wl_if);
			QOSDBG("[BWLIT_GUEST] Processor [%s] Interface \n", wl_if);

			fprintf(f, "\n"
				   "\ttc qdisc del dev %s root 2>/dev/null\n", wl_if);
			fprintf(f, "\tGUEST%d%d=%s\n", i, j, wl_if);
			fprintf(f, "\tTQA%d%d=\"tc qdisc add dev $GUEST%d%d\"\n", i, j, i, j);
			fprintf(f, "\tTCA%d%d=\"tc class add dev $GUEST%d%d\"\n", i, j, i, j);
			fprintf(f, "\tTFA%d%d=\"tc filter add dev $GUEST%d%d\"\n", i, j, i, j); // 5
#if defined(RTCONFIG_SOC_IPQ8074)
			fprintf(f, "\n"
				   "\t$TQA%d%d root handle %d: htb default %d\n", i, j, guest, guest_mark);
#else
			fprintf(f, "\n"
				   "\t$TQA%d%d root handle %d: htb\n", i, j, guest);
#endif
			fprintf(f, "\t$TCA%d%d parent %d: classid %d:1 htb rate %skbit\n", i, j, guest, guest, nvram_pf_safe_get(wlv, "_bw_dl")); //7
			fprintf(f, "\n"
				   "\t$TCA%d%d parent %d:1 classid %d:%d htb rate 1kbit ceil %skbit prio %d\n", i, j, guest, guest, guest_mark, nvram_pf_safe_get(wlv, "_bw_dl"), guest_mark);
			fprintf(f, "\t$TQA%d%d parent %d:%d handle %d: $SCH\n", i, j, guest, guest_mark, guest_mark);
			fprintf(f, "\t$TFA%d%d parent %d: prio %d protocol ip u32 match mark %d 0x%x flowid %d:%d\n", i, j, guest, guest_mark, guest_mark, QOS_MASK, guest, guest_mark); // 10
			fprintf(f, "\n"
				   "\t$TCAU parent 2:1 classid 2:%d htb rate 1kbit ceil %skbit prio %d\n", guest_mark, nvram_safe_get(strcat_r(wlv, "_bw_ul", tmp)), guest_mark);
			fprintf(f, "\t$TQAU parent 2:%d handle %d: $SCH\n", guest_mark, guest_mark);
			fprintf(f, "\t$TFAU parent 2: prio %d protocol ip u32 match mark %d 0x%x flowid 2:%d\n", guest_mark, guest_mark, QOS_MASK, guest_mark); //13
			QOSDBG("[BWLIT_GUEST] create %s bandwidth limiter, qdisc=%d, class=%d\n", wl_if, guest, guest_mark);
			guest++; // add guest 3: ~ 14: (12 guestnetwork)
			guest_mark++;
			j++;
		}
		i++; j = 1;
	}

	/* Stop Function */
	fprintf(f,
		"}\n\n"
		"stop()\n"
		"{\n"
		/* Flush ebtables */
		"\t#ebtables -t nat -F\n\n"
		//WAN/LAN
		"\ttc qdisc del dev $WAN root 2>/dev/null\n"
		"\ttc qdisc del dev br0 root 2>/dev/null\n"
		);
	i = 0; j = 1;
	foreach(wl, nvram_safe_get("wl_ifnames"), next) {
		SKIP_ABSENT_BAND_AND_INC_UNIT(i);
		snprintf(prefix, sizeof(prefix), "wl%d_", i);
		foreach(wlv, nvram_safe_get(strcat_r(prefix, "vifnames", tmp)), next2) {
			if (!nvram_pf_get_int(wlv, "_bss_enabled") ||
			    !nvram_pf_get_int(wlv, "_bw_enabled")) {
				j++;
				continue;
			}
			wl_if = wl_ifname;
			get_wlxy_ifname(i, j, wl_if);
			fprintf(f, "\ttc qdisc del dev %s root 2>/dev/null\n", wl_if);
			j++;
		}
		i++; j = 1;
	}

	/* Show Function */
	fprintf(f,
		"}\n\n"
		"show()\n"
		"{\n"
		"\ttc -s -d class ls dev $WAN\n"
		"\ttc -s -d class ls dev br0\n"
		);

	/* Main Funtion */
	fprintf(f,
		"}\n\n"
		"if [ $# != 1 ]; then\n"
		"\techo \"Usage: $0 start/stop/restart\"\n"
		"else\n"
		"\tif [ $1 = \"start\" ]; then\n"
		"\t\tstart\n"
		"\telif [ $1 = \"stop\" ]; then\n"
		"\t\tstop\n"
		"\telif [ $1 = \"restart\" ]; then\n"
		"\t\tstop\n"
		"\t\tstart\n"
		"\tfi\n"
		"fi\n"
		);
	fclose(f);

	chmod(qosfn, 0700);
	eval((char *)qosfn, "start");
	QOSDBG("[BWLIT] Execute Bandwidth Limiter Done.\n");

exit_start_bandwidth_limiter:
	file_unlock(lock);

	return ret;
}

static int add_rog_qos_rules(char *pcWANIF)
{
	FILE *fn;
#ifdef RTCONFIG_IPV6
	FILE *fn_ipv6 = NULL;
#endif
	char *buf;
	char *g;
	char *p, *wan;
	char *addr;
	int class_num;
	int unit;
	char main_rule[192], end[256], end2[256];
	char chain[sizeof("QOSOXXX")];
	int v4v6_ok;
	int lock;
	int evalRet;
	char *action = NULL;
	int model = get_model();

	switch (model) {
		case MODEL_DSLAX82U:
			action = "--set-mark";
			manual_return = 1;
			break;
		default:
			action = "--set-return";
			manual_return = 0;
			break;
	}

	del_iQosRules(); // flush related rules in mangle table

	if((fn = fopen(mangle_fn, "w")) == NULL) return -2;
#ifdef RTCONFIG_IPV6
	if(ipv6_enabled() && (fn_ipv6 = fopen(mangle_fn_ipv6, "w")) == NULL){
		fclose(fn);
		return -3;
	}
#endif

	lock = file_lock(qos_ipt_lock);
	QOSDBG("[qos] iptables START\n");
	fprintf(fn,
		"*mangle\n"
		":PREROUTING ACCEPT [0:0]\n"
		":OUTPUT ACCEPT [0:0]\n"
		);
#ifdef RTCONFIG_IPV6
	if (fn_ipv6 && ipv6_enabled())
	fprintf(fn_ipv6,
		"*mangle\n"
		":PREROUTING ACCEPT [0:0]\n"
		":OUTPUT ACCEPT [0:0]\n"
		);
#endif

	for (unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
		wan = get_wan_ifname(unit);
		if (!wan || *wan == '\0')
			continue;
		if (wan_primary_ifunit() != unit
#if defined(RTCONFIG_DUALWAN)
		    && (nvram_match("wans_mode", "fo") || nvram_match("wans_mode", "fb"))
#endif
		)
			continue;
		fprintf(fn, ":QOSO%d - [0:0]\n", unit);
#ifdef RTCONFIG_IPV6
		if (fn_ipv6 && ipv6_enabled())
		fprintf(fn_ipv6, ":QOSO%d - [0:0]\n" , unit);
#endif
	}

	for (unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
		wan = get_wan_ifname(unit);
		if (!wan || *wan == '\0')
			continue;
		if (wan_primary_ifunit() != unit
#if defined(RTCONFIG_DUALWAN)
		    && (nvram_match("wans_mode", "fo") || nvram_match("wans_mode", "fb"))
#endif
		   )
			continue;

		/* Beginning of the Rule */
		/* device */
		class_num = QOS_ROG_MARK_MID;
		snprintf(chain, sizeof(chain), "QOSO%d", unit);	// chain name
		snprintf(end , sizeof(end), " -j MARK %s 0x%x/0x%x\n", action, class_num, QOS_MASK);
		snprintf(end2, sizeof(end2), " -j RETURN\n");

		g = buf = strdup(nvram_safe_get("rog_clientlist"));
		while (g) {

			if ((p = strsep(&g, "<")) == NULL) break;
			if((vstrsep(p, ">", &addr)) != 1) continue;

			v4v6_ok = IPT_V4;
#ifdef RTCONFIG_IPV6
			if (fn_ipv6 && ipv6_enabled())
				v4v6_ok |= IPT_V6;
#endif

			if(strcmp(addr, "")) {
				snprintf(main_rule, sizeof(main_rule), "-m mac --mac-source %s", addr);
			}
			else {
				continue;
			}

			/* build final rule */
			if (v4v6_ok & IPT_V4){
				fprintf(fn, "-A %s %s %s", chain, main_rule, end);
				if(manual_return)
				fprintf(fn, "-A %s %s %s", chain, main_rule, end2);
			}

#ifdef RTCONFIG_IPV6
			if (fn_ipv6 && ipv6_enabled() && (v4v6_ok & IPT_V6)){
				fprintf(fn_ipv6, "-A %s %s %s", chain, main_rule, end);
				if(manual_return)
				fprintf(fn_ipv6, "-A %s %s %s", chain, main_rule, end2);
			}
#endif
		}
		free(buf);

		/* Highest */
		class_num = QOS_ROG_MARK_HIGH;
		snprintf(end , sizeof(end), " -j MARK %s 0x%x/0x%x\n", action, class_num, QOS_MASK);
		snprintf(main_rule , sizeof(main_rule), "-p udp -m multiport --dports 53,67,68");
		fprintf(fn, "-A %s %s %s", chain, main_rule, end);
		if(manual_return)
			fprintf(fn , "-A %s %s %s\n", chain, main_rule, end2);
		snprintf(main_rule , sizeof(main_rule), "-p 1");
		fprintf(fn, "-A %s %s %s", chain, main_rule, end);
		if(manual_return)
			fprintf(fn , "-A %s %s %s\n", chain, main_rule, end2);

		/* The default class */
		class_num = QOS_ROG_MARK_LOW;
		snprintf(end , sizeof(end), " -j MARK %s 0x%x/0x%x\n", action, class_num, QOS_MASK);

		fprintf(fn, "-A %s %s", chain, end);
		if(manual_return)
			fprintf(fn , "-A %s %s\n", chain, end2);
		fprintf(fn, "-A FORWARD -o %s -j %s\n", wan, chain);
		fprintf(fn, "-A OUTPUT -o %s -j %s\n", wan, chain);

#ifdef RTCONFIG_IPV6
		if (fn_ipv6 && ipv6_enabled() && *wan6face && wan_primary_ifunit() == unit) {
			fprintf(fn_ipv6, "-A %s %s", chain, end);
			if(manual_return)
				fprintf(fn_ipv6, "-A %s %s\n", chain, end2);
			fprintf(fn_ipv6, "-A FORWARD -o %s -j %s\n", wan6face, chain);
			fprintf(fn_ipv6, "-A OUTPUT -o %s -j %s\n", wan6face, chain);
		}
#endif
	} /* for (u = WAN_UNIT_FIRST; u < WAN_UNIT_MAX; ++u) */

	fprintf(fn, "COMMIT\n");
	fclose(fn);
	chmod(mangle_fn, 0700);
	evalRet = eval("iptables-restore", "-n", (char*)mangle_fn);
	rule_apply_checking("qos_multiwan", __LINE__, (char*)mangle_fn, evalRet);
#ifdef RTCONFIG_IPV6
	if (fn_ipv6 && ipv6_enabled())
	{
		fprintf(fn_ipv6, "COMMIT\n");
		fclose(fn_ipv6);
		chmod(mangle_fn_ipv6, 0700);

//		eval("ip6tables-restore", "-n", (char*)mangle_fn_ipv6);
//		ipv6_qos_applied++;
	}
#endif
	file_unlock(lock);
	QOSDBG("[rog] iptables DONE!\n");

	return 0;
}

static int start_rog_qos()
{
	static char wan_if_list[WAN_UNIT_MAX][IFNAMSIZ] = { "", "" };
	int ret = 0, gen_mangle = 0;
	FILE *f, *f_top;
	int lock;
	char *wan;
	int unit;
	char fname[sizeof(TMP_QOS) + 4];	/* /tmp/qos, /tmp/qos.0, /tmp/qos.1 ... */
	char prefix[16];
	unsigned int obw;
	char obw_str[8];
	char *qsched;
#ifdef HND_ROUTER
	int wantype = get_dualwan_by_unit(wan_primary_ifunit());
#endif

	/* If WAN interface changes, generate mangle table again. */
	for (unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
		if (wan_primary_ifunit() != unit
#if defined(RTCONFIG_DUALWAN)
		    && (nvram_match("wans_mode", "fo") || nvram_match("wans_mode", "fb"))
#endif
		   )
			continue;
		wan = get_wan_ifname(unit);
		if (!wan || *wan == '\0' || !strcmp(wan, wan_if_list[unit]))
			continue;

		sprintf(prefix, "wan%d_", unit);
		if (!nvram_pf_match(prefix, "state_t", "2"))
			continue;
		gen_mangle++;
		strcpy(wan_if_list[unit], wan);
	}
	if (gen_mangle)
		add_rog_qos_rules("");

#ifdef HND_ROUTER
	if (wantype == WANS_DUALWAN_IF_WAN) {
		eval((char *)qosfn, "stop");
		config_obw();
		nvram_set_int("fc_disable", nvram_get_int("fc_disable_force") ? 1 : 0);
		if (nvram_get_int("fc_disable"))
			fc_fini();
		else
			fc_init();
		return 0;
	}
	else {
		nvram_set_int("fc_disable", 1);
		fc_fini();
	}
#endif

	lock = file_lock(qos_lock);

	if (!(f_top = fopen(qosfn, "w"))) {
		ret = -2;
		goto exit_rog_qos;
	}

	/* Create top-level /tmp/qos */
	QOSDBG("[qos] tc START!\n");
	fprintf(f_top,"#!/bin/sh\n");

	/* Create /tmp/qos.X for each WAN unit. */
	for (unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) {
		snprintf(fname, sizeof(fname), "%s.%d", TMP_QOS, unit);
		wan = get_wan_ifname(unit);
		if (!wan || *wan == '\0')
			continue;

		if (wan_primary_ifunit() != unit
#if defined(RTCONFIG_DUALWAN)
		    && (nvram_match("wans_mode", "fo") || nvram_match("wans_mode", "fb"))
#endif
		   )
		{
			if (f_exists(fname))
				unlink(fname);
			/* Remove qdisc of unused WAN. Ref to stop case below. */
			eval("tc", "qdisc", "del", "dev", wan, "root");
			continue;
		}
		if (!(f = fopen(fname, "w"))) {
			fprintf(stderr, "[qos] Can't write to %s\n", fname);
			continue;
		}

#if defined(RTCONFIG_DUALWAN)
		if(unit != 0)
			snprintf(obw_str, sizeof(obw_str), "qos_obw%d", unit);
		else
#endif
			snprintf(obw_str, sizeof(obw_str), "qos_obw");
		obw = strtoul(nvram_safe_get(obw_str), NULL, 10);
		if (!obw)
		{
#ifdef DSL_AX82U
			if (is_ax5400_i1())
			{
				if (unit)
					snprintf(obw_str, sizeof(obw_str), "qos_xobw%d", unit);
				else
					snprintf(obw_str, sizeof(obw_str), "qos_xobw");
				obw = strtoul(nvram_safe_get(obw_str), NULL, 10);
				if (!obw)
					continue;
			}
			else
#endif
			continue;
		}

		fprintf(stderr, "[qos][unit %d] tc START!\n", unit);

		/* Egress OBW  -- set the HTB shaper (Classful Qdisc)
		 * the BW is set here for each class
		 */

#if defined(RTCONFIG_HND_ROUTER_AX)
		qsched = "fq_codel quantum 300 limit 1000 noecn";
#else
		qsched = "sfq perturb 10";
#endif

		/* WAN */
		fprintf(f,
			"#!/bin/sh\n"
			"SCH=\"%s\"\n"
			"I=%s\n"
			"TQA=\"tc qdisc add dev $I\"\n"
			"TCA=\"tc class add dev $I\"\n"
			"TFA=\"tc filter add dev $I\"\n"
			"case \"$1\" in\n"
			"start)\n"
			"\ttc qdisc del dev $I root 2>/dev/null\n"
			"\t$TQA root handle 1: htb default 30\n"
			"\t$TCA parent 1: classid 1:1 htb rate %ukbit ceil %ukbit\n"
			"\t$TCA parent 1:1 classid 1:10 htb rate %ukbit ceil %ukbit prio 1\n"
			"\t$TCA parent 1:1 classid 1:20 htb rate %ukbit ceil %ukbit prio 2\n"
			"\t$TCA parent 1:1 classid 1:30 htb rate %ukbit ceil %ukbit prio 3\n"
			"\t$TQA parent 1:10 handle 10: $SCH\n"
			"\t$TQA parent 1:20 handle 20: $SCH\n"
			"\t$TQA parent 1:30 handle 30: $SCH\n"
			"\t$TFA parent 1: prio 10 protocol all handle %d/0x%x fw flowid 1:10\n"
			"\t$TFA parent 1: prio 20 protocol all handle %d/0x%x fw flowid 1:20\n"
			"\t$TFA parent 1: prio 30 protocol all handle %d/0x%x fw flowid 1:30\n"
			, qsched
			, wan
			, obw, obw
			, obw, obw
			, obw / 10, obw
			, obw / 100, obw
			, QOS_ROG_MARK_HIGH, QOS_MASK
			, QOS_ROG_MARK_MID, QOS_MASK
			, QOS_ROG_MARK_LOW, QOS_MASK
			);

		fputs(
			"\t;;\n"
			"stop)\n"
			"\ttc qdisc del dev $I root 2>/dev/null\n"
			"\t;;\n"
			"*)\n"
			"\t#---------- Upload ----------\n"
			"\ttc -s -d filter ls dev $I\n"
			"\ttc -s -d qdisc ls dev $I\n"
			"\techo\n"
			"esac\n"
			, f);

		fclose(f);
		chmod(fname, 0700);

		fprintf(f_top, "[ -e %s ] && %s \"$1\"\n", fname, fname);
		QOSDBG("[qos]unit %d] tc done!\n", unit);
	} /* for (unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit) */

	fclose(f_top);
	chmod(qosfn, 0700);
	eval((char *)qosfn, "start");
	QOSDBG("[qos] tc done!\n");

exit_rog_qos:
	file_unlock(lock);

	return ret;
}

int add_iQosRules(char *pcWANIF)
{
	int status = 0, qos_type = nvram_get_int("qos_type");

	if (pcWANIF == NULL)
		return -1;

	if (nvram_get_int("qos_enable") != 1) {
		if (nvram_get_int("rog_enable")
#ifdef RTCONFIG_BWDPI
		 && !dump_dpi_support(INDEX_ADAPTIVE_QOS)
#endif
		)
			return add_rog_qos_rules(pcWANIF);
		else
			return -1;
	}

	switch (qos_type) {
	case 0:
		/* Traditional QoS */
		status = add_qos_rules(pcWANIF);
		break;
	case 1:
		/* Adaptive QoS */
		return -1;
	case 2:
		/* Bandwidthh limiter */
		status = add_bandwidth_limiter_rules(pcWANIF);
		break;
	default:
		/* Unknown QoS type */
		_dprintf("%s: unknown qos_type %d, pcWANIF %s\n",
			__func__, qos_type, pcWANIF? pcWANIF : "<nil>");
	}

	if (status < 0)
		_dprintf("[%s] status = %d\n", __func__, status);

	return status;
}

#ifdef RTCONFIG_AMAS_WGN
static int start_bandwidth_limiter_AMAS_WGN(void)
{
	FILE *f = NULL;
	char wl_ifname[IFNAMSIZ];

	if ((f = fopen(qosfn, "w")) == NULL) return -2;

	/* Start Function */
	fprintf(f,
		"#!/bin/sh\n"
		"SFQ=\"sfq perturb 10\"\n"
		"\n"
		"start()\n"
		"{\n"
	);

	// init guest 3: ~ 14: (12 guestnetwork), start number = 3
	guest = 3;
	int  guest_mark = GUEST_INIT_MARKNUM;
	char wl[128], wlv[128], tmp[128], *next, *next2;
	char prefix[32];
	char *wl_if = wl_ifname;
	int  i = 0;
	int  j = 1;

	/* Loop */
	foreach(wl, nvram_safe_get("wl_ifnames"), next) {
		SKIP_ABSENT_BAND_AND_INC_UNIT(i);
		snprintf(prefix, sizeof(prefix), "wl%d_", i);
		foreach(wlv, nvram_safe_get(strcat_r(prefix, "vifnames", tmp)), next2) {
			if (!nvram_pf_get_int(wlv, "_bss_enabled") ||
			    !nvram_pf_get_int(wlv, "_bw_enabled")) {
				j++;
				continue;
			}
			WGN_ifname(i, j, wl_if);
			QOSDBG("[BWLIT_GUEST] Processor [%s] Interface \n", wl_if);
			if (!strcmp(wl_if, "")) continue;

			fprintf(f, "\n"
				   "\ttc qdisc del dev %s root 2>/dev/null\n", wl_if);
			fprintf(f, "\tGUEST%d%d=%s\n", i, j, wl_if);
			fprintf(f, "\tTQA%d%d=\"tc qdisc add dev $GUEST%d%d\"\n", i, j, i, j);
			fprintf(f, "\tTCA%d%d=\"tc class add dev $GUEST%d%d\"\n", i, j, i, j);
			fprintf(f, "\tTFA%d%d=\"tc filter add dev $GUEST%d%d\"\n", i, j, i, j); // 5
			fprintf(f, "\n"
				   "\t$TQA%d%d root handle %d: htb default %d\n", i, j, guest, guest_mark);
			fprintf(f, "\t$TCA%d%d parent %d: classid %d:1 htb rate %skbit\n", i, j, guest, guest, nvram_pf_safe_get(wlv, "_bw_dl")); //7
			fprintf(f, "\n"
				   "\t$TCA%d%d parent %d:1 classid %d:%d htb rate 1kbit ceil %skbit prio %d\n", i, j, guest, guest, guest_mark, nvram_pf_safe_get(wlv, "_bw_dl"), guest_mark);
			fprintf(f, "\t$TQA%d%d parent %d:%d handle %d: $SFQ\n", i, j, guest, guest_mark, guest_mark);
			fprintf(f, "\t$TFA%d%d parent %d: prio %d protocol ip u32 match mark %d 0x%x flowid %d:%d\n", i, j, guest, guest_mark, guest_mark, QOS_MASK, guest, guest_mark); // 10
			QOSDBG("[BWLIT_GUEST] create %s bandwidth limiter, qdisc=%d, class=%d\n", wl_if, guest, guest_mark);

			guest++; // add guest 3: ~ 14: (12 guestnetwork)
			guest_mark++;
			j++;
		}
		i++; j = 1;
	}
	
	/* Stop Function */
	fprintf(f,
		"}\n\n"
		"stop()\n"
		"{\n"
		/* Flush ebtables */
		"\t#ebtables -t nat -F\n\n"
		);

	/* Loop */
	i = 0; j = 1;
	foreach(wl, nvram_safe_get("wl_ifnames"), next) {
		SKIP_ABSENT_BAND_AND_INC_UNIT(i);
		snprintf(prefix, sizeof(prefix), "wl%d_", i);
		foreach(wlv, nvram_safe_get(strcat_r(prefix, "vifnames", tmp)), next2) {
			if(nvram_get_int(strcat_r(wlv, "_bss_enabled", tmp)) && 
			   nvram_get_int(strcat_r(wlv, "_bw_enabled" , tmp))) {
				wl_if = wl_ifname;
				WGN_ifname(i, j, wl_if);
				if (!strcmp(wl_if, "")) continue;
				fprintf(f, "\ttc qdisc del dev %s root 2>/dev/null\n", wl_if);
			}
			j++;
		}
		i++; j = 1;
	}
	
	/* Show Function */
	fprintf(f,
		"}\n\n"
		"show()\n"
		"{\n"
		);

	/* Loop */
	i = 0; j = 1;
	foreach(wl, nvram_safe_get("wl_ifnames"), next) {
		SKIP_ABSENT_BAND_AND_INC_UNIT(i);
		snprintf(prefix, sizeof(prefix), "wl%d_", i);
		foreach(wlv, nvram_safe_get(strcat_r(prefix, "vifnames", tmp)), next2) {
			if(nvram_get_int(strcat_r(wlv, "_bss_enabled", tmp)) && 
			   nvram_get_int(strcat_r(wlv, "_bw_enabled" , tmp))) {
				wl_if = wl_ifname;
				WGN_ifname(i, j, wl_if);
				if (!strcmp(wl_if, "")) continue;
				fprintf(f, "\ttc -s -d class ls dev %s\n", wl_if);
			}
			j++;
		}
		i++; j = 1;
	}
	
	/* Main Funtion */
	fprintf(f,
		"}\n\n"
		"if [ $# != 1 ]; then\n"
		"\techo \"Usage: $0 start/stop/restart\"\n"
		"else\n"
		"\tif [ $1 = \"start\" ]; then\n"
		"\t\tstart\n"
		"\telif [ $1 = \"stop\" ]; then\n"
		"\t\tstop\n"
		"\telif [ $1 = \"restart\" ]; then\n"
		"\t\tstop\n"
		"\t\tstart\n"
		"\tfi\n"
		"fi\n"
		);

	fclose(f);
	chmod(qosfn, 0700);
	eval((char *)qosfn, "start");
	QOSDBG("[BWLIT_RE] Execute Bandwidth Limiter Done.\n");

	return 0;
}
#endif

int start_iQos(void)
{
	int status = 0, qos_type = nvram_get_int("qos_type");

#ifdef DSL_AX82U
	if (is_ax5400_i1()) config_obw();
#endif

	if (nvram_get_int("qos_enable") != 1) {
		if (nvram_get_int("rog_enable")
#ifdef RTCONFIG_BWDPI
		 && !dump_dpi_support(INDEX_ADAPTIVE_QOS)
#endif
		)
			return start_rog_qos();
		else
			return -1;
	}

	switch (qos_type) {
	case 0:
		/* Traditional QoS */
		status = start_tqos();
		break;
	case 1:
		/* Adaptive QoS */
		return -1;
	case 2:
		/* Bandwidthh limiter */
		// AMAS non-RE mode
		if (nvram_get_int("re_mode") == 0)
			status = start_bandwidth_limiter();
#ifdef RTCONFIG_AMAS_WGN
		// AMAS RE mode
		if (nvram_get_int("re_mode") == 1)
			status = start_bandwidth_limiter_AMAS_WGN();
#endif
		break;
	default:
		/* Unknown QoS type */
		_dprintf("%s: unknown qos_type %d\n",
			__func__, qos_type);
	}

	if (status < 0)
		_dprintf("[%s] status = %d\n", __func__, status);

	return status;
}

int check_wl_guest_bw_enable()
{
	char wl[128], wlv[128], tmp[128], *next, *next2;
	char prefix[32];
	int  i = 0;

	foreach(wl, nvram_safe_get("wl_ifnames"), next) {
		SKIP_ABSENT_BAND_AND_INC_UNIT(i);
		snprintf(prefix, sizeof(prefix), "wl%d_", i);
		foreach(wlv, nvram_safe_get(strcat_r(prefix, "vifnames", tmp)), next2) {
			
			if ( nvram_get_int(strcat_r(wlv, "_bss_enabled", tmp)) && 
			     nvram_get_int(strcat_r(wlv, "_bw_enabled" , tmp))) {
				return 1;
			}
		}
		i++;
	}
	return 0;
}

void ForceDisableWLan_bw(void)
{
	char wl[128], wlv[128], tmp[128], *next, *next2;
	char prefix[32];
	int  i = 0;

	foreach(wl, nvram_safe_get("wl_ifnames"), next) {
		SKIP_ABSENT_BAND_AND_INC_UNIT(i);
		snprintf(prefix, sizeof(prefix), "wl%d_", i);
		foreach(wlv, nvram_safe_get(strcat_r(prefix, "vifnames", tmp)), next2) {
			nvram_set_int(strcat_r(wlv, "_bw_enabled" , tmp), 0);
		}
		i++;
	}
	QOSDBG("[BWLIT] ALL Guest Netwok of Bandwidth Limiter has been Didabled.\n");
}
