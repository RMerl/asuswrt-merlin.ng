 /*
 * Copyright 2022, ASUSTeK Inc.
 * All Rights Reserved.
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND ASUS GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. ASUS
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */

/*
	feature implement:
	1. traditaional qos
	2. bandwdith limiter (also for guest network)
	3. facebook wifi     (already EOL in the end of 2017)
	4. GeForceNow qos

	NOTE:
	qos mark bit 8~31 : TrendMicro adaptive qos usage, so ASUS only can use bit 0~7 for different applications
	ex. Traditional qos / bandwidth limiter / Facebook wifi / GeForceNow QoS
*/

#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "rc.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#ifdef RTCONFIG_BWDPI
#include <bwdpi.h>
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

static const char *qosfn = "/tmp/qos";
static const char *mangle_fn = "/tmp/mangle_rules";
#ifdef RTCONFIG_IPV6
static const char *mangle_fn_ipv6 = "/tmp/mangle_rules_ipv6";
#endif

int etable_flag = 0;
int manual_return = 0;

static int qos_action_manual()
{
	int ret;
	ret = 1;

	return ret;
}

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

static void add_iptables_AMAS_WGN(FILE *fn, const char *action)
{
	/* Setup guest network's ebtables rules */
	int  guest_mark = GUEST_INIT_MARKNUM;
	char wl[128] = {0}, wlv[128] = {0}, tmp[128] = {0}, *next = NULL, *next2 = NULL;
	char prefix[32] = {0};
	char mssid_mark[4] = {0};
	int  i = 0;
	int  j = 1;
	char net[64] = {0};

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
				wl_vif_to_subnet(wlv, net, sizeof(net)); // shared/misc.c
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

/*
	ip / mac / ip-range status
*/
enum {
	TYPE_UNKNOWN = -1,
	TYPE_IP = 0,
	TYPE_MAC,
	TYPE_IPRANGE
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
	1. 192.168.1.*    = 192.168.1.1-254           (subnet)
	2. 192.168.1.0/24 = 192.168.1.1-192.168.1.254 (subnet)
	3. 192.168.1.10-20                            (short)
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
			goto END;
		}
		g += len_to_dot + 1;
	}

	/* copy head */
	strncpy(head, old, len_total);

	/* case1 : x.x.x.* subnet */
	if (*g == '*') {
		snprintf(new, len, "%s1-%s254", head, head);
		ret = 1;
		goto END;
	}

	/* case2 : IP subnetting, x.x.x.0/24 */
	p = NULL;
	p = strchr(g, '/');
	if (p != NULL) {
		len_to_line = p - g;
		memset(a, 0, sizeof(a));
		strncpy(a, g, len_to_line);

		// validate value is valid IP num
		if (isIPnum(a) == -1) {
			QOSLOG("case 2: p+=%s, g=%s, a=%s", p+1, g, a);
			goto END;
		}

		// get mask and mask_addr
		g += len_to_dot + 1;
		mask = isSubnet(p+1);
		if (mask == 0) goto END;
		snprintf(host, sizeof(host), "%s%s", head, a);
		mask_t = ntohl(inet_addr(host));
		mask_addr = mask_t & (0xffffffff & (0xffffffff << (32 - mask)));
		host_start = mask_addr + 1;
		host_end = mask_addr + (0xffffffff & ~(0xffffffff << (32 - mask))) - 1;
		inet_src.s_addr = htonl(host_start);
		inet_dst.s_addr = htonl(host_end);

		QOSLOG("case 2: mask=%d, mask_addr=%x, host_start=%x, host_end=%x", mask, mask_addr, host_start, host_end);

		start = inet_ntoa(inet_src);
		strncat(new, start, strlen(start));
		strncat(new, "-", 1);
		end = inet_ntoa(inet_dst);
		strncat(new, end, strlen(end));
		QOSLOG("case 2: new=%s, end=%s", new, end);
		ret = 1;
		goto END;
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
			goto END;
		}

		snprintf(new, len, "%s%s-%s%s", head, a, head, (p+1));
		ret = 1;
		goto END;
	}

END:
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

	memset(s, 0, sizeof(s));

	// mac format
	g = buf = strdup(old);
	if (sscanf(g, "%02X:%02X:%02X:%02X:%02X:%02X",&s[0],&s[1],&s[2],&s[3],&s[4],&s[5]) == 6) {
#ifdef RTCONFIG_AMAS
		QOSLOG("address_format_checker");
		if (amas_lib_device_ip_query(old, new)) {
			*type = TYPE_IP;
			QOSLOG("is_ip=%d, is_mac=%d, is_range=%d, type=%d, new=%s", is_ip, is_mac, is_range, *type, new);
			return;
		} else
#endif
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
	else {
		*type = TYPE_UNKNOWN;
		strncpy(new, "", len);
	}
	QOSLOG("is_ip=%d, is_mac=%d, is_range=%d, type=%d, new=%s", is_ip, is_mac, is_range, *type, new);
}

static unsigned calc(unsigned bw, unsigned pct)
{
	unsigned n = ((unsigned long)bw * pct) / 100;
	return (n < 2) ? 2 : n;
}

void del_EbtablesRules(void)
{
	/* Flush all rules in nat table of ebtable*/
	if (module_loaded("ebtable_nat"))
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

	for (band = 0; band < min(MAX_NR_WL_IF, (sizeof(fbwifi_iface)/sizeof(fbwifi_iface[0]))); ++band) {
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
	int UnitNum = 2; 			// wl0.x, wl1.x
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
	if(sw_mode() == SW_MODE_ROUTER){
		set_fbwifi_mark();
	}
#endif

	etable_flag = 1;
}
#endif

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
				WGN_ifname(i, j, wl_if);
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
#ifdef CLS_ACT
	eval("ip", "link", "set", "imq0", "down");
#endif

	del_EbtablesRules(); // flush ebtables nat table

	/* Flush all rules in mangle table */
	eval("iptables", "-t", "mangle", "-F");
#ifdef RTCONFIG_IPV6
	eval("ip6tables", "-t", "mangle", "-F");
#endif

#if 0
#ifdef RTCONFIG_BCMARM
	remove_codel_patch();
#endif
#endif
}

static int add_qos_rules(char *pcWANIF)
{
	FILE *fn;
#ifdef RTCONFIG_IPV6
	FILE *fn_ipv6 = NULL;
#endif
	char *buf;
	char *g;
	char *p;
	char *desc, *addr, *port, *prio, *transferred, *proto;
	int class_num;
	int class_mask=7;
	int lan_class_num=6; 	// for LAN-to-LAN class_num = 0x6 / 0x16
	int class_gum = 16; 	// 0x10
	const char *lan_ifname = nvram_safe_get("lan_ifname");
	int i, inuse;
	char q_inuse[32]; 	// for inuse
	char dport[256], saddr_1[192], proto_1[8], proto_2[8],conn[256], end[256], end2[256];
	//int method;
	int sticky_enable;
	const char *chain;
	int v4v6_ok;

	if((fn = fopen(mangle_fn, "w")) == NULL) return -2;
#ifdef RTCONFIG_IPV6
	if(ipv6_enabled() && (fn_ipv6 = fopen(mangle_fn_ipv6, "w")) == NULL){
		fclose(fn);
		return -3;
	}
#endif

	inuse = sticky_enable = 0;

	/* action and manual_return */
	char *action = NULL;
	int evalRet;

	if (qos_action_manual() == 0) {
		action = "--set-return";
		manual_return = 0;
	}
	else if (qos_action_manual() == 1) {
		action = "--set-mark";
		manual_return = 1;
	}

	if(nvram_match("qos_sticky", "0"))
		sticky_enable = 1;

	del_iQosRules(); // flush all rules in mangle table

#ifdef CLS_ACT
	eval("ip", "link", "set", "imq0", "up");
#endif
	QOSDBG("[qos] iptables START\n");

	fprintf(fn,
		"*mangle\n"
		":PREROUTING ACCEPT [0:0]\n"
		":OUTPUT ACCEPT [0:0]\n"
		":QOSO - [0:0]\n"
		"-A QOSO -j CONNMARK --restore-mark --mask 0x%x\n"
		"-A QOSO -m connmark ! --mark 0/0x%x -j RETURN\n",
			class_mask,
			class_gum
		);
#ifdef RTCONFIG_IPV6
	if (fn_ipv6 && ipv6_enabled())
	fprintf(fn_ipv6,
		"*mangle\n"
		":PREROUTING ACCEPT [0:0]\n"
		":OUTPUT ACCEPT [0:0]\n"
		":QOSO - [0:0]\n"
		"-A QOSO -j CONNMARK --restore-mark --mask 0x%x\n"
		"-A QOSO -m connmark ! --mark 0/0x%x -j RETURN\n",
			class_mask,
			class_gum
		);
#endif

	const char *lan_ipaddr = nvram_safe_get("lan_ipaddr");
	const char *lan_netmask = nvram_safe_get("lan_netmask");
#ifdef CONFIG_BCMWL5 // TODO: it is only for the case, eth0 as wan, vlanx as lan
	if(strncmp(pcWANIF, "ppp", 3)==0){
		// ppp related interface doesn't need physdev
		// do nothing
	}
	else{
		// for LAN-to-LAN
		// note : iptables will happily apply the given netmask and store the base network address; no need to do that ourself
		fprintf(fn,
			"-A QOSO -s %s/%s -d %s/%s -j CONNMARK %s 0x%x/0x%x\n",
				lan_ipaddr, lan_netmask, lan_ipaddr, lan_netmask, action, lan_class_num|class_gum, class_mask|class_gum
			);
		if(manual_return)
			fprintf(fn,
				"-A QOSO -s %s/%s -d %s/%s -j RETURN\n",
					lan_ipaddr, lan_netmask, lan_ipaddr, lan_netmask
				);
		// for multicast
		fprintf(fn,
			"-A QOSO -d 224.0.0.0/4 -j CONNMARK %s 0x%x/0x%x\n",
				action, lan_class_num|class_gum, class_mask|class_gum
			);
		if(manual_return)
			fprintf(fn, "-A QOSO -d 224.0.0.0/4 -j RETURN\n");
	}
#endif

#ifdef RTCONFIG_IPV6
	const char *ipv6_prefix = nvram_safe_get("ipv6_prefix");
	const char *ipv6_prefix_length = nvram_safe_get("ipv6_prefix_length");
	if (fn_ipv6 && ipv6_enabled() && *wan6face) {
#ifdef CONFIG_BCMWL5 // TODO: it is only for the case, eth0 as wan, vlanx as lan
		if(strncmp(wan6face, "ppp", 3)==0){
			// ppp related interface doesn't need physdev
			// do nothing
		}
		else{
			// for LAN-to-LAN
			fprintf(fn_ipv6,
				"-A QOSO -s %s/%s -d %s/%s -j CONNMARK %s 0x%x/0x%x\n",
					ipv6_prefix, ipv6_prefix_length, ipv6_prefix, ipv6_prefix_length, action, lan_class_num|class_gum, class_mask|class_gum
				);
			if(manual_return)
				fprintf(fn_ipv6,
					"-A QOSO -s %s/%s -d %s/%s -j RETURN\n",
						ipv6_prefix, ipv6_prefix_length, ipv6_prefix, ipv6_prefix_length
					);
			// for multicast
			fprintf(fn_ipv6,
				"-A QOSO -d ff00::/8 -j CONNMARK %s 0x%x/0x%x\n",
					action, lan_class_num|class_gum, class_mask|class_gum
				);
			if(manual_return)
				fprintf(fn_ipv6, "-A QOSO -d ff00::/8 -j RETURN\n");
		}
#endif
	}
#endif

	g = buf = strdup(nvram_safe_get("qos_rulelist"));
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
		class_num = safe_atoi(prio);
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
			if transferred != NULL, class_num must bt 0x1~0x6, not 0x11~0x16
			0x1~0x6		: keep tracing this connection.
			0x11~0x16 	: connection will be considered as marked connection, won't detect again.
		*/
		int rule_gum = (strcmp(transferred,"") == 0) ? class_gum : 0;

		chain = "QOSO";		// chain name
		snprintf(end , sizeof(end), " -j CONNMARK %s 0x%x/0x%x\n", action, class_num|rule_gum, class_mask|rule_gum);	// CONNMARK string
		snprintf(end2, sizeof(end2), " -j RETURN\n");

		/*************************************************/
		/*                        addr                   */
		/*           src mac or src ip or IP range       */
		/*************************************************/
		char addr_new[40];
		int addr_type;
		memset(addr_new, 0, sizeof(addr_new));

		/* note : if transferred bytes accounting is used (see below),
		          only the source-matched packets will be accounted for */
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
		else if (strcmp(transferred,"") == 0){
			/* note : max number of multiple port in iptables is 15 */
			snprintf(dport, sizeof(dport), "-m multiport --dport %s", port);
		}
		else{
			/* note : for transferred bytes accounting (see below), we must
			          catch both request (dport) and response (sport) packets */
			snprintf(dport, sizeof(dport), "-m multiport --port %s", port);
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

		strlcpy(tmp, transferred, sizeof (tmp));
		tmp_trans = tmp;
		q_min = strsep(&tmp_trans, "~");
		q_max = tmp_trans;

		if (strcmp(transferred,"") == 0){
			snprintf(conn, sizeof(conn), "%s", "");
		}
		else{
			min = atol(q_min);

			if (strcmp(q_max,"") == 0) // q_max == NULL
				snprintf(conn, sizeof(conn), "-m connbytes --connbytes %ld:%s --connbytes-dir both --connbytes-mode bytes", min*1024, q_max);
			else{// q_max != NULL
				max = atol(q_max);
				snprintf(conn, sizeof(conn), "-m connbytes --connbytes %ld:%ld --connbytes-dir both --connbytes-mode bytes", min*1024, max*1024-1);
			}
		}
		QOSLOG("[qos] transferred=%s, min=%ld, max=%ld, q_max=%s, conn=%s", transferred, min*1024, max*1024-1, q_max, conn);

		/*************************************************/
		/*                      proto                    */
		/*        tcp, udp, tcp/udp, any, (icmp, igmp)   */
		/*************************************************/
		memset(proto_1, 0, sizeof(proto_1));
		memset(proto_2, 0, sizeof(proto_2));
		if(!strcmp(proto, "tcp"))
		{
			snprintf(proto_1, sizeof(proto_1), "-p tcp");
			snprintf(proto_2, sizeof(proto_2), "NO");
		}
		else if(!strcmp(proto, "udp"))
		{
			snprintf(proto_1, sizeof(proto_1), "-p udp");
			snprintf(proto_2, sizeof(proto_2), "NO");
		}
		else if(!strcmp(proto, "any"))
		{
			snprintf(proto_1, sizeof(proto_1), "%s", "");
			snprintf(proto_2, sizeof(proto_2), "NO");
		}
		else if(!strcmp(proto, "tcp/udp"))
		{
			snprintf(proto_1, sizeof(proto_1), "-p tcp");
			snprintf(proto_2, sizeof(proto_1), "-p udp");
		}
		else{
			snprintf(proto_1, sizeof(proto_1), "NO");
			snprintf(proto_2, sizeof(proto_2), "NO");
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

	/* The default class */
	i = nvram_get_int("qos_default");
	if ((i < 0) || (i > 4)) i = 3;  // "lowest"
	class_num = i + 1;

#ifdef CONFIG_BCMWL5 // TODO: it is only for the case, eth0 as wan, vlanx as lan
	if(strncmp(pcWANIF, "ppp", 3)==0){
		// ppp related interface doesn't need physdev
		// do nothing
	}
	else{
		/* for WLAN to LAN bridge packet */
		// ebtables : identify bridge packet
		add_EbtablesRules();

		// for download
		fprintf(fn, "-A POSTROUTING -o %s -j QOSO\n", lan_ifname);
	}
#endif
		fprintf(fn,
			"-A QOSO -j CONNMARK %s 0x%x/0x%x\n"
			"-A POSTROUTING -o %s -j QOSO\n",
				action, class_num|class_gum, class_mask|class_gum,
				pcWANIF
			);

#ifdef RTCONFIG_IPV6
	if (fn_ipv6 && ipv6_enabled() && *wan6face) {
#ifdef CONFIG_BCMWL5 // TODO: it is only for the case, eth0 as wan, vlanx as lan
		if(strncmp(wan6face, "ppp", 3)==0){
			// ppp related interface doesn't need physdev
			// do nothing
		}
		else{
			/* for WLAN to LAN bridge packet */
			// ebtables : identify bridge packet
			add_EbtablesRules();

			// for download
			fprintf(fn_ipv6, "-A POSTROUTING -o %s -j QOSO\n", lan_ifname);
		}
#endif
		fprintf(fn_ipv6,
			"-A QOSO -j CONNMARK %s 0x%x/0x%x\n"
			"-A POSTROUTING -o %s -j QOSO\n",
				action, class_num|class_gum, class_mask|class_gum,
				wan6face
			);
	}
#endif

	inuse |= (1 << i) | 1;  // default and highest are always built
	snprintf(q_inuse, sizeof(q_inuse), "%d", inuse);
	nvram_set("qos_inuse", q_inuse);
	QOSDBG("[qos] qos_inuse=%d\n", inuse);

	/* Ingress rules */
	g = buf = strdup(nvram_safe_get("qos_irates"));
	for (i = 0; i < 10; ++i) {
		if ((!g) || ((p = strsep(&g, ",")) == NULL)) continue;
		if ((inuse & (1 << i)) == 0) continue;
		if (safe_atoi(p) > 0) {
			fprintf(fn, "-A PREROUTING -i %s -j CONNMARK --restore-mark --mask 0x%x\n", pcWANIF, class_mask);
#ifdef CLS_ACT
			fprintf(fn, "-A PREROUTING -i %s -j IMQ --todev 0\n", pcWANIF);
#endif
#ifdef RTCONFIG_IPV6
			if (fn_ipv6 && ipv6_enabled() && *wan6face) {
				fprintf(fn_ipv6, "-A PREROUTING -i %s -j CONNMARK --restore-mark --mask 0x%x\n", wan6face, class_mask);
#ifdef CLS_ACT
				fprintf(fn_ipv6, "-A PREROUTING -i %s -j IMQ --todev 0\n", wan6face);
#endif
			}
#endif
			break;
		}
	}
	free(buf);

	fprintf(fn, "COMMIT\n");
	fclose(fn);

	chmod(mangle_fn, 0700);
	evalRet = eval("iptables-restore", (char*)mangle_fn);
	rule_apply_checking("qos", __LINE__, (char*)mangle_fn, evalRet);
#ifdef RTCONFIG_IPV6
	if (fn_ipv6 && ipv6_enabled())
	{
		fprintf(fn_ipv6, "COMMIT\n");
		fclose(fn_ipv6);
		chmod(mangle_fn_ipv6, 0700);
		eval("ip6tables-restore", (char*)mangle_fn_ipv6);
	}
#endif
	run_custom_script("qos-start", 0, "rules", NULL);
	QOSDBG("[qos] iptables DONE!\n");

	return 0;
}


/*******************************************************************/
// The definations of all partations
// eth0 : WAN
// 1:1  : upload (WAN egress)
// 1:2  : download   (10240000Kbits = 10Gbps)
// 1:10 : highest
// 1:20 : high
// 1:30 : middle
// 1:40 : low        (default)
// 1:50 : lowest
// 1:60 : ALL Download (WAN to LAN and LAN to LAN) (10240000kbits = 10Gbps)
// br0  : LAN
// 2:1  : download (WAN ingress -> LAN egress)
// etc.
// WARNING: ingress qdisc (packets) can NOT use (yet unset) iptables marks!
//
/*******************************************************************/

/* Tc */
static int start_tqos(void)
{
	int i;
	char *buf, *g, *p;
	unsigned int rate;
	unsigned int ceil;
	unsigned int ibw, obw, bw;
	unsigned int mtu;
	FILE *f;
	int x;
	int inuse;
	char s[256];
	int first;
	char burst_root[32];
	char burst_leaf[32];
	char *qsched;
	int overhead = 0, overheaddl = 0;
	char overheadstr[sizeof("overhead 128 linklayer ethernet")];
	char overheaddlstr[sizeof("overhead 128 linklayer ethernet")];
	char nvmtu[sizeof("wan0_mtu")];

	// judge interface by get_wan_ifname
	// add Qos iptable rules in mangle table,
	// move it to firewall - mangle_setting
	// add_iQosRules(get_wan_ifname(wan_primary_ifunit())); // iptables start

	ibw = strtoul(nvram_safe_get("qos_ibw"), NULL, 10);
	obw = strtoul(nvram_safe_get("qos_obw"), NULL, 10);
	if(ibw==0||obw==0) return -1;

	if((f = fopen(qosfn, "w")) == NULL) return -2;

	QOSDBG("[qos] tc START!\n");

	/* qos_burst */
	i = nvram_get_int("qos_burst0");
	if(i > 0) snprintf(burst_root, sizeof(burst_root), "burst %dk", i);
		else burst_root[0] = 0;
	i = nvram_get_int("qos_burst1");

	if(i > 0) snprintf(burst_leaf, sizeof(burst_leaf), "burst %dk", i);
		else burst_leaf[0] = 0;

	const char *mode;
	switch (nvram_get_int("qos_atm")) {
	    default:
		mode = "";
		break;
	    case 1:
		mode = " linklayer atm";
		break;
	    case 2:
		mode = "";
		obw = obw * 64 / 65;
		ibw = ibw * 64 / 65;
		break;
	}

	/* Egress OBW  -- set the HTB shaper (Classful Qdisc)
	* the BW is set here for each class
	*/

	bw = obw;

#ifdef RTCONFIG_BCMARM
	if (bw < 51200)
		qsched = "fq_codel quantum 300 limit 1000 noecn";
	else
		qsched = "fq_codel noecn";

	overhead = overheaddl = nvram_get_int("qos_overhead");
#else
	qsched = "sfq perturb 10";
#endif

	const char *wan_ifname = get_wan_ifname(wan_primary_ifunit());
	const char *lan_ifname = nvram_safe_get("lan_ifname");

	if(strncmp(wan_ifname, "ppp", 3)==0) {
		// No adjustment required
	} else {
		// Adjust from Cake-like IP overhead as seen in GUI to what htb uses
		// (it already counts the Ethernet header, so it's not part of its overhead)
		overhead -= 14;
	}
	if (overhead < 0)
		overhead = 0;

#ifdef CLS_ACT
	// No adjustment required for imq0
#else
	// Again, adjust for the Ethernet header on the LAN interface
	overheaddl -= 14;
#endif
	if (overheaddl < 0)
		overheaddl = 0;

#ifdef RTCONFIG_BCMARM
	snprintf(overheadstr, sizeof(overheadstr),"overhead %d %s",
		 overhead, mode);
	snprintf(overheaddlstr, sizeof(overheaddlstr),"overhead %d %s",
		 overheaddl, mode);
#else
	strcpy(overheadstr, "");
	strcpy(overheaddlstr, "");
#endif

	snprintf(nvmtu, sizeof (nvmtu), "wan%d_mtu", wan_primary_ifunit());
	mtu = nvram_get_int(nvmtu);
	if (mtu == 0)
		mtu = 1500;

	/* Upload (WAN egress) */
	fprintf(f,
		"#!/bin/sh\n"
		"#LAN/WAN\n"
		"SCH=\"%s\"\n"
		"ULIF='%s'\n"
		"TQAUL=\"tc qdisc add dev $ULIF\"\n"
		"TCAUL=\"tc class add dev $ULIF\"\n"
		"TFAUL=\"tc filter add dev $ULIF\"\n"
#ifdef CLS_ACT
		"DLIF='imq0'\n"
#else
		"DLIF='%s'\n"
#endif
		"TQADL=\"tc qdisc add dev $DLIF\"\n"
		"TCADL=\"tc class add dev $DLIF\"\n"
		"TFADL=\"tc filter add dev $DLIF\"\n"
		"case \"$1\" in\n"
		"start)\n"
		"# reset\n"
		"\ttc qdisc del dev $ULIF root 2>/dev/null\n"
		"\ttc qdisc del dev $DLIF root 2>/dev/null\n"
		"\n"
		"# upload (root/default)\n"
		"\t$TQAUL root handle 1: htb default %u\n"
		"# upload 1:1\n"
		"\t$TCAUL parent 1: classid 1:1 htb rate %ukbit ceil %ukbit %s %s\n" ,
			qsched,
			wan_ifname,
#ifndef CLS_ACT
			lan_ifname,
#endif
			(nvram_get_int("qos_default") + 1) * 10,
			bw, bw, burst_root, overheadstr
		);

	/* LAN protocol: 802.1q */
#ifdef CONFIG_BCMWL5 // TODO: it is only for the case, eth0 as wan, vlanx as lan
	fprintf(f,
		"# upload 1:2: LAN-to-LAN (vlan@%s)\n"
		"\t$TCAUL parent 1: classid 1:2 htb rate 1000000kbit ceil 1000000kbit burst 10000 cburst 10000\n"
		"# upload 1:60: LAN-to-LAN (vlan@%s)\n"
		"\t$TCAUL parent 1:2 classid 1:60 htb rate 1000000kbit ceil 1000000kbit burst 10000 cburst 10000 prio 6\n"
		"\t$TQAUL parent 1:60 handle 60: pfifo\n"
		"\t$TFAUL parent 1: prio 6 protocol 802.1q u32 match u32 0 0 flowid 1:60\n",
			wan_ifname,
			wan_ifname
		);
#endif

	inuse = nvram_get_int("qos_inuse");

	g = buf = strdup(nvram_safe_get("qos_orates"));
	for (i = 0; i < 5; ++i) { // 0~4 , 0:highest, 4:lowest

		if ((!g) || ((p = strsep(&g, ",")) == NULL)) break;

		if ((inuse & (1 << i)) == 0){
			QOSDBG("[qos] egress %d doesn't create, inuse=%d\n", i, inuse);
			continue;
		}
		else {
			QOSDBG("[qos] egress %d creates\n", i);
		}

		if ((sscanf(p, "%u-%u", &rate, &ceil) != 2) || (rate < 1)) continue;

		if (ceil > 0) snprintf(s, sizeof(s), "ceil %ukbit ", calc(bw, ceil));
			else s[0] = 0;
		x = (i + 1) * 10;

		fprintf(f, "# egress %d: %u-%u%%\n", i, rate, ceil);
		fprintf(f, "\t$TCAUL parent 1:1 classid 1:%d htb rate %ukbit %s %s prio %d quantum %u %s\n", x, calc(bw, rate), s, burst_leaf, (i >= 6) ? 7 : (i + 1), mtu, overheadstr);
		fprintf(f, "\t$TQAUL parent 1:%d handle %d: $SCH\n", x, x);
		fprintf(f, "\t$TFAUL parent 1: prio %d u32 match mark %d 0x%x flowid 1:%d\n", x, i + 1, QOS_MASK, x);
	}
// add for test
#if 1
		fprintf(f, "\t$TFA parent 1: prio 10 u32 match ip protocol 17 0xff match ip tos 0xC8 0xff flowid 1:10\n");
#endif
	free(buf);

	/*
		10000 = ACK
		00100 = RST
		00010 = SYN
		00001 = FIN
	*/

	if (nvram_match("qos_ack", "on")) {
		fprintf(f,
			"# upload: TCP ACK\n"
			"\t$TFAUL parent 1: prio 14 protocol ip u32 "
			"match ip protocol 6 0xff "			// TCP
			"match u8 0x05 0x0f at 0 "			// IP header length
			"match u16 0x0000 0xffc0 at 2 "			// total length (0-63)
			"match u8 0x10 0xff at 33 "			// ACK only
			"flowid 1:10\n");
	}
	if (nvram_match("qos_syn", "on")) {
		fprintf(f,
			"# upload: TCP SYN\n"
			"\t$TFAUL parent 1: prio 15 protocol ip u32 "
			"match ip protocol 6 0xff "			// TCP
			"match u8 0x05 0x0f at 0 "			// IP header length
			"match u16 0x0000 0xffc0 at 2 "			// total length (0-63)
			"match u8 0x02 0x02 at 33 "			// SYN,*
			"flowid 1:10\n");
	}
	if (nvram_match("qos_fin", "on")) {
		fprintf(f,
			"# upload: TCP FIN\n"
			"\t$TFAUL parent 1: prio 17 protocol ip u32 "
			"match ip protocol 6 0xff "			// TCP
			"match u8 0x05 0x0f at 0 "			// IP header length
			"match u16 0x0000 0xffc0 at 2 "			// total length (0-63)
			"match u8 0x01 0x01 at 33 "			// FIN,*
			"flowid 1:10\n");
	}
	if (nvram_match("qos_rst", "on")) {
		fprintf(f,
			"# upload: TCP RST\n"
			"\t$TFAUL parent 1: prio 19 protocol ip u32 "
			"match ip protocol 6 0xff "			// TCP
			"match u8 0x05 0x0f at 0 "			// IP header length
			"match u16 0x0000 0xffc0 at 2 "			// total length (0-63)
			"match u8 0x04 0x04 at 33 "			// RST,*
			"flowid 1:10\n");
	}
	if (nvram_match("qos_icmp", "on")) {
		fputs("# upload: ICMP\n\t$TFAUL parent 1: prio 13 protocol ip u32 match ip protocol 1 0xff flowid 1:10\n", f);
	}

	// Download (WAN ingress)
	first = 1;
	bw = ibw;

	if (bw > 0) {
		g = buf = strdup(nvram_safe_get("qos_irates"));
		for (i = 0; i < 5; ++i) { // 0~4 , 0:highest, 4:lowest
			if ((!g) || ((p = strsep(&g, ",")) == NULL)) break;
			if ((inuse & (1 << i)) == 0) continue;
			if ((rate = safe_atoi(p)) < 1) continue;	// 0 = off

			if (first) {
				first = 0;
				fprintf(f,
					"\n"
					"# download (root/default)\n"
					"\t$TQADL root handle 2: htb default %u\n"
					"# download 2:1\n"
					"\t$TCADL parent 2: classid 2:1 htb rate %ukbit ceil %ukbit %s %s\n"
					"# download 2:2: LAN-to-LAN\n"
					"\t$TCADL parent 2: classid 2:2 htb rate 10240000kbit ceil 10240000kbit burst 10000 cburst 10000\n"
					"# download 2:60: LAN-to-LAN\n"
					"\t$TCADL parent 2:2 classid 2:60 htb rate 10240000kbit ceil 10240000kbit burst 10000 cburst 10000 prio 6\n"
					"\t$TQADL parent 2:60 handle 60: pfifo\n"
					"\t$TFADL parent 2: prio 6 u32 match mark 6 7 flowid 2:60\n",
						(nvram_get_int("qos_default") + 1) * 10,
						bw, bw, burst_root, overheaddlstr
					);
			}

			// rate in kb/s
			unsigned int u = calc(bw, rate);

			// burst rate
			unsigned int v = u / 2;
			if (v < 50) v = 50;

			x = (i + 1) * 10;
			fprintf(f,
				"# download 2:%d: %u%%\n"
				"\t$TCADL parent 2:1 classid 2:%d htb rate %ukbit %s prio %d quantum %u %s\n"
				"\t$TQADL parent 2:%d handle %d: $SCH\n"
				"\t$TFADL parent 2: prio %d u32 match mark %d 7 flowid 2:%d\n",
					i, rate,
					x, calc(bw, rate), burst_leaf, (i >= 6) ? 7 : (i + 1), mtu, overheaddlstr,
					x, x,
					x, i + 1, x);
		}

// add for test
#if 1
		fprintf(f, "\t$TFADL parent 2: prio 10 u32 match ip protocol 17 0xff match ip tos 0xC8 0xff flowid 2:10\n");
#endif

		free(buf);

		if (nvram_match("qos_ack", "on")) {
			fprintf(f,
				"# download: TCP ACK\n"
				"\t$TFADL parent 2: prio 14 protocol ip u32 "
				"match ip protocol 6 0xff "			// TCP
				"match u8 0x05 0x0f at 0 "			// IP header length
				"match u16 0x0000 0xffc0 at 2 "			// total length (0-63)
				"match u8 0x10 0xff at 33 "			// ACK only
				"flowid 2:10\n");
		}
		if (nvram_match("qos_syn", "on")) {
			fprintf(f,
				"# download: TCP SYN\n"
				"\t$TFADL parent 2: prio 15 protocol ip u32 "
				"match ip protocol 6 0xff "			// TCP
				"match u8 0x05 0x0f at 0 "			// IP header length
				"match u16 0x0000 0xffc0 at 2 "			// total length (0-63)
				"match u8 0x02 0x02 at 33 "			// SYN,*
				"flowid 2:10\n");
		}
		if (nvram_match("qos_fin", "on")) {
			fprintf(f,
				"# download: TCP FIN\n"
				"\t$TFADL parent 2: prio 17 protocol ip u32 "
				"match ip protocol 6 0xff "			// TCP
				"match u8 0x05 0x0f at 0 "			// IP header length
				"match u16 0x0000 0xffc0 at 2 "			// total length (0-63)
				"match u8 0x01 0x01 at 33 "			// FIN,*
				"flowid 2:10\n");
		}
		if (nvram_match("qos_rst", "on")) {
			fprintf(f,
				"# download: TCP RST\n"
				"\t$TFADL parent 2: prio 19 protocol ip u32 "
				"match ip protocol 6 0xff "			// TCP
				"match u8 0x05 0x0f at 0 "			// IP header length
				"match u16 0x0000 0xffc0 at 2 "			// total length (0-63)
				"match u8 0x04 0x04 at 33 "			// RST,*
				"flowid 2:10\n");
		}
		if (nvram_match("qos_icmp", "on")) {
			fputs("# download: ICMP\n\t$TFADL parent 2: prio 13 protocol ip u32 match ip protocol 1 0xff flowid 2:10\n", f);
		}
	}

	fputs(
		"\t;;\n"
		"stop)\n"
		"\ttc qdisc del dev $ULIF root 2>/dev/null\n"
		"\ttc qdisc del dev $DLIF root 2>/dev/null\n"
		"\t;;\n"
		"*)\n"
		"\t#---------- Upload ----------\n"
		"\ttc -s -d qdisc ls dev $ULIF\n"
		"\ttc -s -d class ls dev $ULIF\n"
		"\ttc -s -d filter ls dev $ULIF\n"
		"\techo\n"
		"\t#--------- Download ---------\n"
		"\ttc -s -d qdisc ls dev $DLIF\n"
		"\ttc -s -d class ls dev $DLIF\n"
		"\ttc -s -d filter ls dev $DLIF\n"
		"\techo\n"
		"esac\n",
		f);

	fclose(f);
	chmod(qosfn, 0700);
	run_custom_script("qos-start", 120, "init", NULL);
	eval((char *)qosfn, "start");
	QOSDBG("[qos] tc done!\n");

	return 0;
}


void stop_iQos(void)
{
	eval((char *)qosfn, "stop");

#ifdef RTCONFIG_GEFORCENOW
	nvgfn_kernel_setting(0);
	nvgfn_mcs_isauto(0);
#endif
#ifdef HND_ROUTER
	config_obw_off();
#endif
/* OPPO */
#if 0
#ifdef RTCONFIG_ROUTERBOOST 	
	if (!IS_RB_QOS()
#ifdef RTCONFIG_AMAS
			|| (aimesh_re_node() == 1 && nvram_get_int("qos_type") == 4)
#endif			
	) {
	    system("rmmod deDupTx");
	    stop_asus_rbd();
	}
#endif
#endif

}

static int add_bandwidth_limiter_rules(char *pcWANIF)
{
	FILE *fn = NULL;
	char *buf, *g, *p;
	char *enable, *addr, *dlc, *upc, *prio;
	char lan_addr[32];
	char addr_new[40];
	int addr_type;
	char *action = NULL;
	int evalRet;

	if ((fn = fopen(mangle_fn, "w")) == NULL) return -2;
	del_iQosRules(); // flush all rules in mangle table

	if (qos_action_manual() == 0) {
		action = "CONNMARK --set-return";
		manual_return = 0;
	}
	else if (qos_action_manual() == 1) {
		action = "MARK --set-mark";
		manual_return = 1;
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

	snprintf(lan_addr, sizeof(lan_addr), "%s/%s", nvram_safe_get("lan_ipaddr"), nvram_safe_get("lan_netmask"));

	fprintf(fn,
		"*mangle\n"
		":PREROUTING ACCEPT [0:0]\n"
		":OUTPUT ACCEPT [0:0]\n"
		);

	// access router : mark 9
	fprintf(fn,
		"-A POSTROUTING -s %s -d %s -j %s 9\n"
		"-A PREROUTING -s %s -d %s -j %s 9\n"
		, nvram_safe_get("lan_ipaddr"), lan_addr, action
		, lan_addr, nvram_safe_get("lan_ipaddr"), action
		);
	
	if(manual_return){
	fprintf(fn,
		"-A POSTROUTING -s %s -d %s -j RETURN\n"
		"-A PREROUTING -s %s -d %s -j RETURN\n"
		, nvram_safe_get("lan_ipaddr"), lan_addr
		, lan_addr, nvram_safe_get("lan_ipaddr")
		);
	}

	g = buf = strdup(nvram_safe_get("qos_bw_rulelist"));
	while (g) {
		if ((p = strsep(&g, "<")) == NULL) break;
		if ((vstrsep(p, ">", &enable, &addr, &dlc, &upc, &prio)) != 5) continue;
		if (!strcmp(enable, "0")) continue;
		memset(addr_new, 0, sizeof(addr_new));
		address_format_checker(&addr_type, addr, addr_new, sizeof(addr_new));
		QOSDBG("[BWLIT] addr_type=%d, addr=%s, add_new=%s, lan_addr=%s\n", addr_type, addr, addr_new, lan_addr);

		if (addr_type == TYPE_IP){
			fprintf(fn,
				"-A POSTROUTING ! -s %s -d %s -j %s %d\n"
				"-A PREROUTING -s %s ! -d %s -j %s %d\n"
				, lan_addr, addr_new, action, safe_atoi(prio)+INITIAL_MARKNUM
				, addr_new, lan_addr, action, safe_atoi(prio)+INITIAL_MARKNUM
				);
			if(manual_return){
			fprintf(fn,
				"-A POSTROUTING ! -s %s -d %s -j RETURN\n"
				"-A PREROUTING -s %s ! -d %s -j RETURN\n"
				, lan_addr, addr_new, addr_new, lan_addr
				);
			}
		}
		else if (addr_type == TYPE_MAC){
			fprintf(fn,
				"-A PREROUTING -m mac --mac-source %s ! -d %s  -j %s %d\n"
				, addr_new, lan_addr, action, safe_atoi(prio)+INITIAL_MARKNUM
				);
			if(manual_return){
			fprintf(fn,
				"-A PREROUTING -m mac --mac-source %s ! -d %s  -j RETURN\n"
				, addr_new, lan_addr
				);
			}
		}
		else if (addr_type == TYPE_IPRANGE){
			fprintf(fn,
				"-A POSTROUTING ! -s %s -m iprange --dst-range %s -j %s %d\n"
				"-A PREROUTING -m iprange --src-range %s ! -d %s -j %s %d\n"
				, lan_addr, addr_new, action, safe_atoi(prio)+INITIAL_MARKNUM
				, addr_new, lan_addr, action, safe_atoi(prio)+INITIAL_MARKNUM
				);
			if(manual_return){
			fprintf(fn,
				"-A POSTROUTING ! -s %s -m iprange --dst-range %s -j RETURN\n"
				"-A PREROUTING -m iprange --src-range %s ! -d %s -j RETURN\n"
				, lan_addr, addr_new, addr_new, lan_addr
				);
			}
		}
	}
	free(buf);

	// AMAS non-RE mode
	if (nvram_get_int("re_mode") == 0) add_iptables_AMAS_WGN(fn, action);

	fprintf(fn, "COMMIT\n");
	fclose(fn);
	chmod(mangle_fn, 0700);
	evalRet = eval("iptables-restore", (char*)mangle_fn);
	rule_apply_checking("qos", __LINE__, (char*)mangle_fn, evalRet);
	QOSDBG("[BWLIT] Create iptables rules done.\n");

	/* Setup guest network's ebtables rules */
	add_EbtablesRules_BW();

	return 0;
}

static int guest; // qdisc root only 3: ~ 14: (12 guestnetwork)

static int start_bandwidth_limiter(void)
{
	FILE *f = NULL;
	char *buf, *g, *p;
	char *enable, *addr, *dlc, *upc, *prio;
	int class = 0;
	int s[6]; // strip mac address
	int addr_type;
	char addr_new[40];
	char wl_ifname[IFNAMSIZ];
	char *qsched;

#ifdef RTCONFIG_BCMARM
	qsched = "fq_codel quantum 300 limit 1000 noecn";
#else
	qsched = "sfq perturb 10";
#endif

	if ((f = fopen(qosfn, "w")) == NULL) return -2;
	fprintf(f,
		"#!/bin/sh\n"
		"SCH=\"%s\"\n"
		"WAN=%s\n"
		"tc qdisc del dev $WAN root 2>/dev/null\n"
		"tc qdisc del dev $WAN ingress 2>/dev/null\n"
		"tc qdisc del dev br0 root 2>/dev/null\n"
		"tc qdisc del dev br0 ingress 2>/dev/null\n"
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
		, qsched
		, get_wan_ifname(wan_primary_ifunit())
	);
	// access router : mark 9
	// default : 10Gbps
	fprintf(f,
		"\n"
		"\t$TCA parent 1:1 classid 1:9 htb rate 10240000kbit ceil 10240000kbit prio 1\n"
		"\t$TQA parent 1:9 handle 9: $SCH\n"
		"\t$TFA parent 1: prio 1 protocol ip handle 9 fw flowid 1:9\n"
		"\n"
		"\t$TCAU parent 2:1 classid 2:9 htb rate 10240000kbit ceil 10240000kbit prio 1\n"
		"\t$TQAU parent 2:9 handle 9: $SCH\n"
		"\t$TFAU parent 2: prio 1 protocol ip handle 9 fw flowid 2:9\n"
	);

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
		class = safe_atoi(prio) + INITIAL_MARKNUM;
		if (addr_type == TYPE_MAC)
		{
			sscanf(addr_new, "%02X:%02X:%02X:%02X:%02X:%02X",&s[0],&s[1],&s[2],&s[3],&s[4],&s[5]);
			fprintf(f,
				"\n"
				"\t$TCA parent 1:1 classid 1:%d htb rate %skbit ceil %skbit prio %d\n"
				"\t$TQA parent 1:%d handle %d: $SCH\n"
				"\t$TFA parent 1: protocol ip prio %d u32 match u16 0x0800 0xFFFF at -2 match u32 0x%02X%02X%02X%02X 0xFFFFFFFF at -12 match u16 0x%02X%02X 0xFFFF at -14 flowid 1:%d"
				"\n"
				"\t$TCAU parent 2:1 classid 2:%d htb rate %skbit ceil %skbit prio %d\n"
				"\t$TQAU parent 2:%d handle %d: $SCH\n"
				"\t$TFAU parent 2: prio %d protocol ip handle %d fw flowid 2:%d\n"
				, class, dlc, dlc, class
				, class, class
				, class, s[2], s[3], s[4], s[5], s[0], s[1], class
				, class, upc, upc, class
				, class, class
				, class, class, class
			);
		}
		else if (addr_type == TYPE_IP || addr_type == TYPE_IPRANGE)
		{
			fprintf(f,
				"\n"
				"\t$TCA parent 1:1 classid 1:%d htb rate %skbit ceil %skbit prio %d\n"
				"\t$TQA parent 1:%d handle %d: $SCH\n"
				"\t$TFA parent 1: prio %d protocol ip handle %d fw flowid 1:%d\n"
				"\n"
				"\t$TCAU parent 2:1 classid 2:%d htb rate %skbit ceil %skbit prio %d\n"
				"\t$TQAU parent 2:%d handle %d: $SCH\n"
				"\t$TFAU parent 2: prio %d protocol ip handle %d fw flowid 2:%d\n"
				, class, dlc, dlc, class
				, class, class
				, class, class, class
				, class, upc, upc, class
				, class, class
				, class, class, class
			);
		}
	}

	if (buf) free(buf);

	// init guest 3: ~ 14: (12 guestnetwork), start number = 3
	guest = 3;
	int  guest_mark = GUEST_INIT_MARKNUM;
	char wl[128], wlv[128], tmp[128], *next, *next2;
	char prefix[32];
	char *wl_if = wl_ifname;
	int  i = 0;
	int  j = 1;
	
	/* Setup guest network's bandwidth limiter */
	foreach(wl, nvram_safe_get("wl_ifnames"), next) {
		SKIP_ABSENT_BAND_AND_INC_UNIT(i);
		snprintf(prefix, sizeof(prefix), "wl%d_", i);
		foreach(wlv, nvram_safe_get(strcat_r(prefix, "vifnames", tmp)), next2) {

			if(nvram_get_int(strcat_r(wlv, "_bss_enabled", tmp)) && 
			   nvram_get_int(strcat_r(wlv, "_bw_enabled" , tmp))) {
				
				get_wlxy_ifname(i, j, wl_if);
				if(get_model()==MODEL_RTAC87U && (i == 1)) {
					if(j == 1) wl_if = "vlan4000";
					if(j == 2) wl_if = "vlan4001";
					if(j == 3) wl_if = "vlan4002";
				}

				QOSDBG("[BWLIT_GUEST] Processor [%s] Interface \n", wl_if);

				fprintf(f,
					"\n"
					"\ttc qdisc del dev %s root 2>/dev/null\n"
					"\tGUEST%d%d=%s\n"
					"\tTQA%d%d=\"tc qdisc add dev $GUEST%d%d\"\n"
					"\tTCA%d%d=\"tc class add dev $GUEST%d%d\"\n"
					"\tTFA%d%d=\"tc filter add dev $GUEST%d%d\"\n" // 5
					"\n"
#if defined(RTCONFIG_QCA)
					"\t$TQA%d%d root handle %d: htb default %d\n"
#else
					"\t$TQA%d%d root handle %d: htb\n"
#endif
					"\t$TCA%d%d parent %d: classid %d:1 htb rate %skbit\n" // 7
					"\n"
					"\t$TCA%d%d parent %d:1 classid %d:%d htb rate 1kbit ceil %skbit prio %d\n"
					"\t$TQA%d%d parent %d:%d handle %d: $SCH\n"
					"\t$TFA%d%d parent %d: prio %d protocol ip handle %d fw flowid %d:%d\n" // 10
					"\n"
					"\t$TCAU parent 2:1 classid 2:%d htb rate 1kbit ceil %skbit prio %d\n"
					"\t$TQAU parent 2:%d handle %d: $SCH\n"
					"\t$TFAU parent 2: prio %d protocol ip handle %d fw flowid 2:%d\n" // 13
					, wl_if
					, i, j, wl_if
					, i, j, i, j
					, i, j, i, j
					, i, j, i, j // 5
#if defined(RTCONFIG_QCA)
					, i, j, guest, guest_mark
#else
					, i, j, guest
#endif
					, i, j, guest, guest, nvram_safe_get(strcat_r(wlv, "_bw_dl", tmp)) //7
					, i, j, guest, guest, guest_mark, nvram_safe_get(strcat_r(wlv, "_bw_dl", tmp)), guest_mark
					, i, j, guest, guest_mark, guest_mark
					, i, j, guest, guest_mark, guest_mark, guest, guest_mark // 10
					, guest_mark, nvram_safe_get(strcat_r(wlv, "_bw_ul", tmp)), guest_mark
					, guest_mark, guest_mark
					, guest_mark, guest_mark, guest_mark //13
				);
				QOSDBG("[BWLIT_GUEST] create %s bandwidth limiter, qdisc=%d, class=%d\n", wl_if, guest, guest_mark);
				guest++; // add guest 3: ~ 14: (12 guestnetwork)
				guest_mark++;
			} //bss_enabled
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
			
			if(nvram_get_int(strcat_r(wlv, "_bss_enabled", tmp)) && 
			   nvram_get_int(strcat_r(wlv, "_bw_enabled" , tmp))) {
				
				wl_if = wl_ifname;
				get_wlxy_ifname(i, j, wl_ifname);
				if(get_model()==MODEL_RTAC87U && (i == 1)) {
					if(j == 1) wl_if = "vlan4000";
					if(j == 2) wl_if = "vlan4001";
					if(j == 3) wl_if = "vlan4002";
				}
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

	return 0;
}

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

#ifdef RTCONFIG_GEFORCENOW
static int nvfgn_GetQoSChannelPort(char *str)
{
	int ret = 0;
	char *buf = NULL, *g = NULL, *p = NULL;
	char *type = NULL, *proto = NULL, *port = NULL;

	g = buf = strdup(nvram_safe_get("nvgfn_ch_rulelist"));
	while (g) {
		if ((p = strsep(&g, "<")) == NULL) break;
		if ((vstrsep(p, ">", &type, &proto, &port)) != 3) continue;
		if (!strcmp(str, type)) {
			ret = atoi(port);
			break;
		}
	}
	if (buf) free(buf);
	return ret;
}

static int start_GeForce_QoS(void)
{
	FILE *f = NULL;
	unsigned int ibw = 0, obw = 0;
	unsigned int ibw_re = 0, obw_re = 0;
	char *qsched;
	int overhead = 0;
	char overheadstr[sizeof("overhead 128 linklayer ethernet")];
	int mtu, bw;
	char nvmtu[sizeof("wan0_mtu")];

	_dprintf("[GeForce] start GeForceNow QoS ...\n");
	ibw = strtoul(nvram_safe_get("qos_ibw"), NULL, 10);
	obw = strtoul(nvram_safe_get("qos_obw"), NULL, 10);
	ibw_re = strtoul(nvram_safe_get("nvgfn_ibw_r"), NULL, 10);
	obw_re = strtoul(nvram_safe_get("nvgfn_obw_r"), NULL, 10);

	/* If this value doesn't exist or 0 or empty, will give 1Gbps as default value */
	if (ibw == 0) ibw = 10240000;
	if (obw == 0) obw = 10240000;
	if (ibw_re == 0) ibw_re = 10240000;
	if (obw_re == 0) obw_re = 10240000;

	/* If this value exist and it is less then 1Mbps, will force into 1Mbps */
	if (ibw < 1024 && ibw != 0) ibw = 1024;
	if (obw < 1024 && obw != 0) obw = 1024;
	if (ibw_re < 1024 && ibw_re != 0) ibw_re = 1024;
	if (obw_re < 1024 && obw_re != 0) obw_re = 1024;

	bw = obw;
#ifdef RTCONFIG_BCMARM
	if (bw < 51200)
		qsched = "fq_codel quantum 300 limit 1000 noecn";
	else
		qsched = "fq_codel noecn";
	overhead = nvram_get_int("qos_overhead");
#else
	qsched = "sfq perturb 10";
#endif

	if (overhead > 0)
		snprintf(overheadstr, sizeof(overheadstr),"overhead %d %s",
		         overhead, nvram_get_int("qos_atm") ? "linklayer atm" : "linklayer ethernet");
	else
		strcpy(overheadstr, "");

	snprintf(nvmtu, sizeof (nvmtu), "wan%d_mtu", wan_primary_ifunit());
	mtu = nvram_get_int(nvmtu);
	if (mtu == 0)
		mtu = 1500;

	if ((f = fopen(qosfn, "w")) == NULL) return -2;
	fprintf(f,
		"#!/bin/sh\n"
		"SCH='%s'\n"
		"WAN=%s\n"
		"LAN=%s\n"
		"\n"
		"TQAU=\"tc qdisc add dev $WAN\"\n"
		"TCAU=\"tc class add dev $WAN\"\n"
		"TFAU=\"tc filter add dev $WAN\"\n"
		"SFQ=\"sfq perturb 10\"\n"
		"TQA=\"tc qdisc add dev $LAN\"\n"
		"TCA=\"tc class add dev $LAN\"\n"
		"TFA=\"tc filter add dev $LAN\"\n"
		"\n"
		, qsched
		, get_wan_ifname(wan_primary_ifunit())
		, nvram_safe_get("lan_ifname")
	);

	fprintf(f,
		"start()\n"
		"{\n"
		"echo \"root ...\"\n"
		"$TQA root handle 1: htb default 50\n"
		"$TCA parent 1: classid 1:1 htb rate %ukbit %s\n"
		"\n"
		"$TQAU root handle 2: htb default 50\n"
		"$TCAU parent 2: classid 2:1 htb rate %ukbit %s\n"
		"\n"
		"echo \"download ...\"\n"
		"# 1:10\n"
		"$TCA parent 1:1 classid 1:10 htb rate %ukbit ceil %ukbit prio 1 quantum %u %s\n"
		"$TQA parent 1:10 handle 10: $SCH\n"
		"$TFA parent 1: prio 1 protocol ip handle 10 fw flowid 1:10\n"
		"\n"
		"# 1:20\n"
		"$TCA parent 1:1 classid 1:20 htb rate %fkbit ceil %ukbit prio 2 quantum %u %s\n"
		"$TQA parent 1:20 handle 20: $SCH\n"
		"$TFA parent 1: prio 2 protocol ip handle 20 fw flowid 1:20\n"
		"\n"
		"# 1:30\n"
		"$TCA parent 1:1 classid 1:30 htb rate %fkbit ceil %ukbit prio 3 quantum %u %s\n"
		"$TQA parent 1:30 handle 30: $SCH\n"
		"$TFA parent 1: prio 3 protocol ip handle 30 fw flowid 1:30\n"
		"\n"
		"# 1:40\n"
		"$TCA parent 1:1 classid 1:40 htb rate %fkbit ceil %ukbit prio 4 quantum %u %s\n"
		"$TQA parent 1:40 handle 40: $SCH\n"
		"$TFA parent 1: prio 4 protocol ip handle 40 fw flowid 1:40\n"
		"\n"
		"# 1:50\n"
		"$TCA parent 1:1 classid 1:50 htb rate %fkbit ceil %ukbit prio 5 quantum %u %s\n"
		"$TQA parent 1:50 handle 50: $SCH\n"
		"$TFA parent 1: prio 5 protocol ip handle 50 fw flowid 1:50\n"
		"\n"
		, ibw,      overheadstr            // 1:
		, obw,      overheadstr            // 2:
		, ibw_re,   ibw, mtu, overheadstr  // 1:10
		, 0.20*ibw, ibw, mtu, overheadstr  // 1:20
		, 0.15*ibw, ibw, mtu, overheadstr  // 1:30
		, 0.10*ibw, ibw, mtu, overheadstr  // 1:40
		, 0.05*ibw, ibw, mtu, overheadstr  // 1:50
	);

	/* fixed ports */
	fprintf(f, "$TFA parent 1: prio 10 protocol ip u32 match ip protocol 17 0xff match ip dport %d 0xffff flowid 1:10\n", nvfgn_GetQoSChannelPort("audio"));
	fprintf(f, "$TFA parent 1: prio 10 protocol ip u32 match ip protocol 17 0xff match ip dport %d 0xffff flowid 1:10\n", nvfgn_GetQoSChannelPort("mic"));
	fprintf(f, "$TFA parent 1: prio 10 protocol ip u32 match ip protocol 17 0xff match ip dport %d 0xffff flowid 1:10\n", nvfgn_GetQoSChannelPort("video"));
	fprintf(f, "$TFA parent 1: prio 10 protocol ip u32 match ip protocol 17 0xff match ip dport %d 0xffff flowid 1:10\n", nvfgn_GetQoSChannelPort("control"));
	fprintf(f, "$TFA parent 1: prio 10 protocol ip u32 match ip protocol  6 0xff match ip dport %d 0xffff flowid 1:10\n", nvfgn_GetQoSChannelPort("control"));

	/* DSCP / TOS */
	fprintf(f, "$TFA parent 1: prio 11 protocol ip u32 match ip protocol 17 0xff match ip tos 0xE0 0xff flowid 1:10\n");
	fprintf(f, "$TFA parent 1: prio 11 protocol ip u32 match ip protocol 17 0xff match ip tos 0xC0 0xff flowid 1:10\n");
	fprintf(f, "$TFA parent 1: prio 11 protocol ip u32 match ip protocol 17 0xff match ip tos 0xB8 0xff flowid 1:10\n");
	fprintf(f, "$TFA parent 1: prio 12 protocol ip u32 match ip protocol 17 0xff match ip tos 0xA0 0xff flowid 1:20\n");
	fprintf(f, "$TFA parent 1: prio 12 protocol ip u32 match ip protocol 17 0xff match ip tos 0x80 0xff flowid 1:20\n");
	fprintf(f, "$TFA parent 1: prio 13 protocol ip u32 match ip protocol 17 0xff match ip tos 0x60 0xff flowid 1:30\n");
	fprintf(f, "$TFA parent 1: prio 13 protocol ip u32 match ip protocol 17 0xff match ip tos 0x00 0xff flowid 1:30\n");
	fprintf(f, "$TFA parent 1: prio 14 protocol ip u32 match ip protocol 17 0xff match ip tos 0x40 0xff flowid 1:40\n");
	fprintf(f, "$TFA parent 1: prio 14 protocol ip u32 match ip protocol 17 0xff match ip tos 0x20 0xff flowid 1:40\n");

	fprintf(f,
		"\n"
		"echo \"upload ...\"\n"
		"# 2:10\n"
		"$TCAU parent 2:1 classid 2:10 htb rate %ukbit ceil %ukbit prio 1 quantum %u %s\n"
		"$TQAU parent 2:10 handle 10: $SCH\n"
		"$TFAU parent 2: prio 1 protocol ip handle 10 fw flowid 2:10\n"
		"\n"
		"# 2:20\n"
		"$TCAU parent 2:1 classid 2:20 htb rate %fkbit ceil %ukbit prio 2 quantum %u %s\n"
		"$TQAU parent 2:20 handle 20: $SCH\n"
		"$TFAU parent 2: prio 2 protocol ip handle 20 fw flowid 2:20\n"
		"\n"
		"# 2:30\n"
		"$TCAU parent 2:1 classid 2:30 htb rate %fkbit ceil %ukbit prio 3 quantum %u %s\n"
		"$TQAU parent 2:30 handle 30: $SCH\n"
		"$TFAU parent 2: prio 3 protocol ip handle 30 fw flowid 2:30\n"
		"\n"
		"# 2:40\n"
		"$TCAU parent 2:1 classid 2:40 htb rate %fkbit ceil %ukbit prio 4 quantum %u %s\n"
		"$TQAU parent 2:40 handle 40: $SCH\n"
		"$TFAU parent 2: prio 4 protocol ip handle 40 fw flowid 2:40\n"
		"\n"
		"# 2:50\n"
		"$TCAU parent 2:1 classid 2:50 htb rate %fkbit ceil %ukbit prio 5 quantum %u %s\n"
		"$TQAU parent 2:50 handle 50: $SCH\n"
		"$TFAU parent 2: prio 5 protocol ip handle 50 fw flowid 2:50\n"
		"\n"
		, obw_re  , obw, mtu, overheadstr   // 2:10
		, 0.20*obw, obw, mtu, overheadstr   // 2:20
		, 0.15*obw, obw, mtu, overheadstr   // 2:30
		, 0.10*obw, obw, mtu, overheadstr   // 2:40
		, 0.05*obw, obw, mtu, overheadstr   // 2:50
	);

	/* fixed ports */
	fprintf(f, "$TFAU parent 2: prio 10 protocol ip u32 match ip protocol 17 0xff match ip sport %d 0xffff flowid 2:10\n", nvfgn_GetQoSChannelPort("audio"));
	fprintf(f, "$TFAU parent 2: prio 10 protocol ip u32 match ip protocol 17 0xff match ip sport %d 0xffff flowid 2:10\n", nvfgn_GetQoSChannelPort("mic"));
	fprintf(f, "$TFAU parent 2: prio 10 protocol ip u32 match ip protocol 17 0xff match ip sport %d 0xffff flowid 2:10\n", nvfgn_GetQoSChannelPort("video"));
	fprintf(f, "$TFAU parent 2: prio 10 protocol ip u32 match ip protocol 17 0xff match ip sport %d 0xffff flowid 2:10\n", nvfgn_GetQoSChannelPort("control"));
	fprintf(f, "$TFAU parent 2: prio 10 protocol ip u32 match ip protocol  6 0xff match ip sport %d 0xffff flowid 2:10\n", nvfgn_GetQoSChannelPort("control"));

	/* DSCP / TOS */
	fprintf(f, "$TFAU parent 2: prio 11 protocol ip u32 match ip protocol 17 0xff match ip tos 0xE0 0xff flowid 2:10\n");
	fprintf(f, "$TFAU parent 2: prio 11 protocol ip u32 match ip protocol 17 0xff match ip tos 0xC0 0xff flowid 2:10\n");
	fprintf(f, "$TFAU parent 2: prio 11 protocol ip u32 match ip protocol 17 0xff match ip tos 0xB8 0xff flowid 2:10\n");
	fprintf(f, "$TFAU parent 2: prio 12 protocol ip u32 match ip protocol 17 0xff match ip tos 0xA0 0xff flowid 2:20\n");
	fprintf(f, "$TFAU parent 2: prio 12 protocol ip u32 match ip protocol 17 0xff match ip tos 0x80 0xff flowid 2:20\n");
	fprintf(f, "$TFAU parent 2: prio 13 protocol ip u32 match ip protocol 17 0xff match ip tos 0x60 0xff flowid 2:30\n");
	fprintf(f, "$TFAU parent 2: prio 13 protocol ip u32 match ip protocol 17 0xff match ip tos 0x00 0xff flowid 2:30\n");
	fprintf(f, "$TFAU parent 2: prio 14 protocol ip u32 match ip protocol 17 0xff match ip tos 0x40 0xff flowid 2:40\n");
	fprintf(f, "$TFAU parent 2: prio 14 protocol ip u32 match ip protocol 17 0xff match ip tos 0x20 0xff flowid 2:40\n");
	fprintf(f, "}\n"); // start() end

	fprintf(f,
		"\n"
		"stop()\n"
		"{\n"
		"echo \"stop ...\"\n"
		"tc qdisc del dev $WAN root 2>/dev/null\n"
		"tc qdisc del dev $WAN ingress 2>/dev/null\n"
		"tc qdisc del dev $LAN root 2>/dev/null\n"
		"tc qdisc del dev $LAN ingress 2>/dev/null\n"
		"}\n"
		"\n"
		"up()\n"
		"{\n"
		"echo \"upload ...\"\n"
		"tc qdisc ls dev $WAN\n"
		"tc class ls dev $WAN\n"
		"tc filter ls dev $WAN\n"
		"}\n"
		"\n"
		"down()\n"
		"{\n"
		"echo \"down ...\"\n"
		"tc qdisc ls dev $LAN\n"
		"tc class ls dev $LAN\n"
		"tc filter ls dev $LAN\n"
		"}\n"
		"\n"
		"if [ $# != 1 ]; then\n"
		"echo \"Usage: $0 start/stop/restart/up/down\"\n"
		"else\n"
		"if [ $1 = \"start\" ]; then\n"
		"start\n"
		"elif [ $1 = \"stop\" ]; then\n"
		"stop\n"
		"elif [ $1 = \"restart\" ]; then\n"
		"stop\n"
		"start\n"
		"elif [ $1 = \"up\" ]; then\n"
		"up\n"
		"elif [ $1 = \"down\" ]; then\n"
		"down\n"
		"fi\n"
		"fi\n"
	);

	fclose(f);
	chmod(qosfn, 0700);
	eval((char *)qosfn, "restart");
	QOSDBG("[GeForce] Execute GeForceNow QoS Done.\n");

	nvgfn_kernel_setting(1);

	return 0;
}
#endif


/* OPPO */
#ifdef RTCONFIG_ROUTERBOOST
static int start_RouterBoost_QoS(void)
{
	FILE *f = NULL;
	unsigned int ibw = 0, obw = 0;
	
	_dprintf("[RB] start RouterBoost QoS ...\n");
	ibw = strtoul(nvram_safe_get("qos_ibw"), NULL, 10);
	obw = strtoul(nvram_safe_get("qos_obw"), NULL, 10);
	
	/* If this value doesn't exist or 0 or empty, will give 1Gbps as default value */
	if (ibw == 0) ibw = 10240000;
	if (obw == 0) obw = 10240000;
	
	/* If this value exist and it is less then 1Mbps, will force into 1Mbps */
	if (ibw < 1024 && ibw != 0) ibw = 1024;
	if (obw < 1024 && obw != 0) obw = 1024;
	
	if ((f = fopen(qosfn, "w")) == NULL) return -2;
	fprintf(f,
		"#!/bin/sh\n"
		"WAN=%s\n"
		"LAN=%s\n"
		"\n"
		"TQAU=\"tc qdisc add dev $WAN\"\n"
		"TCAU=\"tc class add dev $WAN\"\n"
		"TFAU=\"tc filter add dev $WAN\"\n"
		"SFQ=\"sfq perturb 10\"\n"
		"TQA=\"tc qdisc add dev $LAN\"\n"
		"TCA=\"tc class add dev $LAN\"\n"
		"TFA=\"tc filter add dev $LAN\"\n"
		"\n"
		, get_wan_ifname(wan_primary_ifunit())
		, nvram_safe_get("lan_ifname")
	);

	fprintf(f,
		"start()\n"
		"{\n"
		"echo \"root ...\"\n"
		"$TQA root handle 1: htb default 20\n"
		"$TCA parent 1: classid 1:1 htb rate %ukbit\n"
		"\n"
		"$TQAU root handle 2: htb default 20\n"
		"$TCAU parent 2: classid 2:1 htb rate %ukbit\n"
		"\n"
		"echo \"download ...\"\n"
		"# 1:10\n"
		"$TCA parent 1:1 classid 1:10 htb rate %fkbit ceil %ukbit prio 1\n"
		"$TQA parent 1:10 handle 10: $SFQ\n"
		//"$TFA parent 1: prio 1 protocol ip handle 10 fw flowid 1:10\n"
		"\n"
		"# 1:20\n"
		"$TCA parent 1:1 classid 1:20 htb rate %fkbit ceil %ukbit prio 2\n"
		"$TQA parent 1:20 handle 20: $SFQ\n"
		//"$TFA parent 1: prio 2 protocol ip handle 20 fw flowid 1:20\n"
		"\n"
		, ibw             // 1:
		, obw             // 2:
		, 0.20*ibw,  ibw   // 1:10
		, 0.20*ibw, ibw   // 1:20
	);

	/* mark */
	fprintf(f, "$TFA parent 1: prio 10 protocol ip u32 match mark %d 0x%x flowid 1:%d\n", 6, QOS_MASK, 10);

	/* DSCP / TOS  */
	fprintf(f, "$TFA parent 1: prio 1 protocol ip u32 match ip protocol 17 0xff match ip tos 0xb8 0xff flowid 1:10\n");
	fprintf(f, "$TFA parent 1: prio 1 protocol ip u32 match ip protocol 17 0xff match ip tos 0xa0 0xff flowid 1:10\n");
	fprintf(f, "$TFA parent 1: prio 2 protocol ip u32 match ip protocol 17 0xff match ip tos 0x00 0xff flowid 1:20\n");
	
	fprintf(f,
		"\n"
		"echo \"upload ...\"\n"
		"# 2:10\n"
		"$TCAU parent 2:1 classid 2:10 htb rate %fkbit ceil %ukbit prio 1\n"
		"$TQAU parent 2:10 handle 10: $SFQ\n"
		//"$TFAU parent 2: prio 1 protocol ip handle 10 fw flowid 2:10\n"
		"\n"
		"# 2:20\n"
		"$TCAU parent 2:1 classid 2:20 htb rate %fkbit ceil %ukbit prio 2\n"
		"$TQAU parent 2:20 handle 20: $SFQ\n"
		//"$TFAU parent 2: prio 2 protocol ip handle 20 fw flowid 2:20\n"
		"\n"
		, 0.20*obw, obw   // 2:10
		, 0.20*obw, obw   // 2:20
	);

	/* mark */
	fprintf(f, "$TFAU parent 2: prio 1 protocol ip u32 match mark %d 0x%x flowid 2:%d\n", 10, QOS_MASK, 10);

	/* DSCP / TOS */
	fprintf(f, "$TFAU parent 2: prio 1 protocol ip u32 match ip protocol 17 0xff match ip tos 0xc8 0xff flowid 2:10\n");
	fprintf(f, "$TFAU parent 2: prio 2 protocol ip u32 match ip protocol 17 0xff match ip tos 0x00 0xff flowid 2:20\n");
	
	fprintf(f, "}\n"); // start() end

	fprintf(f,
		"\n"
		"stop()\n"
		"{\n"
		"echo \"stop ...\"\n"
		"tc qdisc del dev $WAN root 2>/dev/null\n"
		"tc qdisc del dev $WAN ingress 2>/dev/null\n"
		"tc qdisc del dev $LAN root 2>/dev/null\n"
		"tc qdisc del dev $LAN ingress 2>/dev/null\n"
		"}\n"
		"\n"
		"up()\n"
		"{\n"
		"echo \"upload ...\"\n"
		"tc qdisc ls dev $WAN\n"
		"tc class ls dev $WAN\n"
		"tc filter ls dev $WAN\n"
		"}\n"
		"\n"
		"down()\n"
		"{\n"
		"echo \"down ...\"\n"
		"tc qdisc ls dev $LAN\n"
		"tc class ls dev $LAN\n"
		"tc filter ls dev $LAN\n"
		"}\n"
		"\n"
		"if [ $# != 1 ]; then\n"
		"echo \"Usage: $0 start/stop/restart/up/down\"\n"
		"else\n"
		"if [ $1 = \"start\" ]; then\n"
		"start\n"
		"elif [ $1 = \"stop\" ]; then\n"
		"stop\n"
		"elif [ $1 = \"restart\" ]; then\n"
		"stop\n"
		"start\n"
		"elif [ $1 = \"up\" ]; then\n"
		"up\n"
		"elif [ $1 = \"down\" ]; then\n"
		"down\n"
		"fi\n"
		"fi\n"
	);

	fclose(f);
	chmod(qosfn, 0700);
	eval((char *)qosfn, "restart");
	QOSDBG("[RB] Execute RouterBoost QoS Done.\n");

	
	return 0;
}

#endif

static int add_rog_qos_rules(char *pcWANIF)
{
	FILE *fn;
#ifdef RTCONFIG_IPV6
	FILE *fn_ipv6 = NULL;
#endif
	char *buf;
	char *g;
	char *p;
	char wan[16];
	char *addr;
	int class_num;
	char main_rule[192], end[256], end2[256];
	char chain[sizeof("QOSOXXX")];
	int v4v6_ok;
	int evalRet;
	char *action = NULL;

	if (qos_action_manual() == 0) {
		action = "--set-return";
		manual_return = 0;
	}
	else if (qos_action_manual() == 1) {
		action = "--set-mark";
		manual_return = 1;
	}

	del_iQosRules(); // flush related rules in mangle table

	if((fn = fopen(mangle_fn, "w")) == NULL) return -2;
#ifdef RTCONFIG_IPV6
	if(ipv6_enabled() && (fn_ipv6 = fopen(mangle_fn_ipv6, "w")) == NULL){
		fclose(fn);
		return -3;
	}
#endif

	strlcpy(wan, get_wan_ifname(wan_primary_ifunit()), sizeof(wan));

	QOSDBG("[qos] iptables START\n");
	fprintf(fn,
		"*mangle\n"
		":PREROUTING ACCEPT [0:0]\n"
		":OUTPUT ACCEPT [0:0]\n"
		":QOSO - [0:0]\n"
		);
#ifdef RTCONFIG_IPV6
	if (fn_ipv6 && ipv6_enabled())
	fprintf(fn_ipv6,
		"*mangle\n"
		":PREROUTING ACCEPT [0:0]\n"
		":OUTPUT ACCEPT [0:0]\n"
		":QOSO - [0:0]\n"
		);
#endif

	/* Beginning of the Rule */
	/* device */
	class_num = QOS_ROG_MARK_MID;
	snprintf(chain, sizeof(chain), "QOSO");	// chain name
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

	fprintf(fn, "-A %s -j MARK %s 0x%x/0x%x\n", chain, action, class_num, QOS_MASK);
	if(manual_return)
		fprintf(fn , "-A %s -j RETURN\n", chain);
	fprintf(fn, "-A FORWARD -o %s -j %s\n", wan, chain);
	fprintf(fn, "-A OUTPUT -o %s -j %s\n", wan, chain);

#ifdef RTCONFIG_IPV6
	if (fn_ipv6 && ipv6_enabled() && *wan6face) {
		fprintf(fn_ipv6, "-A %s -j MARK %s 0x%x/0x%x\n", chain, action, class_num, QOS_MASK);
		if(manual_return)
			fprintf(fn_ipv6, "-A %s -j RETURN\n", chain);
		fprintf(fn_ipv6, "-A FORWARD -o %s -j %s\n", wan6face, chain);
		fprintf(fn_ipv6, "-A OUTPUT -o %s -j %s\n", wan6face, chain);
	}
#endif

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
	QOSDBG("[rog] iptables DONE!\n");

	return 0;
}

static int start_rog_qos()
{
	FILE *f;
	char wan[16];
	unsigned int obw;
	char *qsched;
#ifdef HND_ROUTER
	int wantype = get_dualwan_by_unit(wan_primary_ifunit());
#endif
	int mtu, bw;
	int overhead = 0;
	char overheadstr[sizeof("overhead 128 linklayer ethernet")];
	char nvmtu[sizeof("wan0_mtu")];

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

	obw = strtoul(nvram_safe_get("qos_obw"), NULL, 10);
	if (!obw) {
#ifdef DSL_AX82U
		if (is_ax5400_i1()) {
			obw = strtoul(nvram_safe_get("qos_xobw"), NULL, 10);
			if (!obw)
				return -1;
		}
		else
#endif
		return -1;
	}

	bw = obw;

#ifdef RTCONFIG_BCMARM
	if (bw < 51200)
		qsched = "fq_codel quantum 300 limit 1000 noecn";
	else
		qsched = "fq_codel noecn";
	overhead = nvram_get_int("qos_overhead");
#else
	qsched = "sfq perturb 10";
#endif

	if (overhead > 0)
		snprintf(overheadstr, sizeof(overheadstr),"overhead %d %s",
		         overhead, nvram_get_int("qos_atm") ? "linklayer atm" : "linklayer ethernet");
	else
		strcpy(overheadstr, "");

	snprintf(nvmtu, sizeof (nvmtu), "wan%d_mtu", wan_primary_ifunit());
	mtu = nvram_get_int(nvmtu);
	if (mtu == 0)
		mtu = 1500;

	if((f = fopen(qosfn, "w")) == NULL) return -2;

	strlcpy(wan, get_wan_ifname(wan_primary_ifunit()), sizeof(wan));

	/* Create top-level /tmp/qos */
	QOSDBG("[qos] tc START!\n");

	/* Egress OBW  -- set the HTB shaper (Classful Qdisc)
	 * the BW is set here for each class
	 */

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
		"\t$TCA parent 1: classid 1:1 htb rate %ukbit ceil %ukbit %s\n"
		"\t$TCA parent 1:1 classid 1:10 htb rate %ukbit ceil %ukbit prio 1 quantum %u %s\n"
		"\t$TCA parent 1:1 classid 1:20 htb rate %ukbit ceil %ukbit prio 2 quantum %u %s\n"
		"\t$TCA parent 1:1 classid 1:30 htb rate %ukbit ceil %ukbit prio 3 quantum %u %s\n"
		"\t$TQA parent 1:10 handle 10: $SCH\n"
		"\t$TQA parent 1:20 handle 20: $SCH\n"
		"\t$TQA parent 1:30 handle 30: $SCH\n"
		"\t$TFA parent 1: prio 10 protocol all handle %d/0x%x fw flowid 1:10\n"
		"\t$TFA parent 1: prio 20 protocol all handle %d/0x%x fw flowid 1:20\n"
		"\t$TFA parent 1: prio 30 protocol all handle %d/0x%x fw flowid 1:30\n"
		, qsched
		, wan
		, obw, obw, overheadstr
		, obw, obw, mtu, overheadstr
		, obw / 10, obw, mtu, overheadstr
		, obw / 100, obw, mtu, overheadstr
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
	chmod(qosfn, 0700);

	eval((char *)qosfn, "start");
	QOSDBG("[qos] tc done!\n");

	return 0;
}


int add_iQosRules(char *pcWANIF)
{
	int status = 0;

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

	if (pcWANIF == NULL || nvram_get_int("qos_type") == 1) return -1;

	if (IS_TQOS()) {
		status = add_qos_rules(pcWANIF);
	}
	else if (IS_BW_QOS()) {
		status = add_bandwidth_limiter_rules(pcWANIF);
	}
/* OPPO */
#if 0
#ifdef RTCONFIG_ROUTERBOOST
	else if (IS_RB_QOS()
#ifdef RTCONFIG_AMAS
			&&(aimesh_re_node() == 0)
#endif	
	) {
		status = system("insmod deDupTx");
	}
#endif
#endif
	if (status < 0) _dprintf("[%s] status = %d\n", __FUNCTION__, status);

	return status;
}

int start_iQos(void)
{
	int status = 0;

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

	if (nvram_get_int("qos_type") == 1) return -1;

	if (IS_TQOS()) {
		status = start_tqos();
	}
	else if (IS_BW_QOS()) {
		// AMAS non-RE mode
		if (nvram_get_int("re_mode") == 0)
			status = start_bandwidth_limiter();
		// AMAS RE mode
		if (nvram_get_int("re_mode") == 1)
			status = start_bandwidth_limiter_AMAS_WGN();
	}
#ifdef RTCONFIG_GEFORCENOW
	else if (IS_GFN_QOS()) {
		status = start_GeForce_QoS();
	}
#endif
#ifdef HND_ROUTER
	else if (IS_CAKE_QOS()) {
		status = start_cake();
	}
#endif

/* OPPO */
#if 0
#ifdef RTCONFIG_ROUTERBOOST
	else if (IS_RB_QOS()
#ifdef RTCONFIG_AMAS
             &&(aimesh_re_node() == 0)
#endif				
	) {
		start_asus_rbd();
		status = start_RouterBoost_QoS();
	}
#endif
#endif
	if (status < 0) _dprintf("[%s] status = %d\n", __FUNCTION__, status);

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

#if 0
#ifdef RTCONFIG_BCMARM
void set_codel_patch(void)
{
	int sched;

	if (nvram_get_int("qos_type") != 1)
		return;

	sched = nvram_get_int("qos_sched");

	if (!f_exists("/var/lock/qostc") &&
	    ((sched == 1) || (sched == 2))) {
		eval("touch", "/var/lock/qostc");
		mount("/usr/sbin/faketc", "/usr/sbin/tc", NULL, MS_BIND, NULL);
		logmessage("qos", "Applying codel patch");
	}
}

#if 0
void remove_codel_patch(void)
{
	if (f_exists("/var/lock/qostc")) {
		umount2("/usr/sbin/tc",MNT_DETACH);
		unlink("/var/lock/qostc");
		logmessage("qos", "Removing codel patch");
	}
}
#endif
#endif

#endif

#ifdef HND_ROUTER

int start_cake(void)
{
	unsigned int ibw, obw;
	FILE *f;
	char overheadstr[32];
	char ibwstr[32];
	char obwstr[32];
	char *mode;
	char nvnat[16];
	int nat;

	if((f = fopen("/etc/cake-qos.conf", "w")) == NULL) return -2;

	switch (nvram_get_int("qos_atm")) {
		case 0:
			mode = "";
			break;
		case 1:
			mode = "atm";
			break;
		case 2:
			mode = "ptm";
			break;
		default:
			mode = "";
	}

	ibw = strtoul(nvram_safe_get("qos_ibw"), NULL, 10);
	obw = strtoul(nvram_safe_get("qos_obw"), NULL, 10);

	if (ibw == 0)
		*ibwstr = '\0';
	else
		snprintf(ibwstr, sizeof(ibwstr), "bandwidth %dkbit", ibw);

	if (obw == 0)
		*obwstr = '\0';
	else
		snprintf(obwstr, sizeof(obwstr), "bandwidth %dkbit", obw);

	snprintf(overheadstr, sizeof(overheadstr), "overhead %d mpu %d", nvram_get_int("qos_overhead"), nvram_get_int("qos_mpu"));

	const char *wan_ifname = get_wan_ifname(wan_primary_ifunit());

	snprintf(nvnat, sizeof (nvnat), "wan%d_nat_x", wan_primary_ifunit());
	nat = nvram_get_int(nvnat);

	/* Config parameters */
	fprintf(f,
		"#!/bin/sh\n\n"
		"ULIF='%s'\n"
		"DLIF='%s'\n"
		"MIF='ifb4%s'\n"
		"ULBW='%s'\n"
		"DLBW='%s'\n"
		"OVERHEAD='%s'\n"
		"FRAMING='%s'\n"
		"ULPRIOQUEUE='diffserv3'\n"
		"DLPRIOQUEUE='besteffort'\n"
		"ULOPTIONS='%s dual-srchost'\n"
		"DLOPTIONS='%s wash dual-dsthost ingress'\n",

		wan_ifname,
		wan_ifname,
		wan_ifname,
		obwstr,
		ibwstr,
		overheadstr,
		mode,
		(nat ? "nat" : ""),
		(nat ? "nat" : "")
	);

	append_custom_config("cake-qos.conf",f);
	fclose(f);


	if((f = fopen(qosfn, "w")) == NULL) return -2;

	/* Stop/start rules */
	fprintf(f,
		"#!/bin/sh\n"
		"source /etc/cake-qos.conf\n\n"

		"case \"$1\" in\n"
		"start)\n"
		"# Upload\n"
		"\ttc qdisc add dev $ULIF root cake $ULPRIOQUEUE $ULBW $OVERHEAD $FRAMING $ULOPTIONS 2>/dev/null\n\n"

		"# Download\n"
		"\tip link add name $MIF type ifb 2>/dev/null\n"
		"\ttc qdisc add dev $DLIF handle ffff: ingress 2>/dev/null\n"
		"\ttc qdisc add dev $MIF root cake $DLPRIOQUEUE $DLBW $OVERHEAD $FRAMING $DLOPTIONS 2>/dev/null\n"
		"\tip link set $MIF up 2>/dev/null\n"
		"\ttc filter add dev $DLIF parent ffff: prio 10 matchall action mirred egress redirect dev $MIF 2>/dev/null\n\n"

		"\t;;\n"
		"stop)\n"
		"\ttc qdisc del dev $ULIF root 2>/dev/null\n"
		"\ttc qdisc del dev $DLIF ingress 2>/dev/null\n"
		"\ttc qdisc del dev $MIF root 2>/dev/null\n"
		"\tip link set $MIF down 2>/dev/null\n"
		"\tip link del dev $MIF 2>/dev/null\n"
		"\t;;\n"
		"*)\n"
		"esac\n");

	fclose(f);
	chmod("/etc/cake-qos.conf", 0755);
	chmod(qosfn, 0755);
	run_custom_script("qos-start", 120, "init", NULL);
	eval((char *)qosfn, "start");

	return 0;
}
#endif
