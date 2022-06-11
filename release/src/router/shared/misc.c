/*

	Tomato Firmware
	Copyright (C) 2006-2009 Jonathan Zarate

*/
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <syslog.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <sys/sysinfo.h>
#include <limits.h>		//PATH_MAX, LONG_MIN, LONG_MAX
#ifdef HND_ROUTER
#include <limits.h>
#include <time.h>
#elif !defined (__GLIBC__) && !defined(__UCLIBC__)
#include <limits.h>		//PATH_MAX, LONG_MIN, LONG_MAX
#endif
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/un.h>
#include <sys/mman.h>
#include <asm/byteorder.h>

#include <bcmnvram.h>
#include <bcmdevs.h>
#include <wlutils.h>

#include "shutils.h"
#include "shared.h"
#include "wlif_utils.h"
#include "iboxcom.h"

#ifndef ETHER_ADDR_LEN
#define	ETHER_ADDR_LEN		6
#endif

#ifndef MULTICAST_BIT
#define MULTICAST_BIT  0x0001
#endif

#if defined(RTCONFIG_QCA)
#include <qca.h>
#endif

#if (defined(RTCONFIG_RALINK) || defined(RTCONFIG_QCA))
#else
#include <wlioctl.h>
#endif
#if defined(RTCONFIG_COOVACHILLI)
#define MAC_FMT "%.2X:%.2X:%.2X:%.2X:%.2X:%.2X"
#define MAC_ARG(x) (x)[0],(x)[1],(x)[2],(x)[3],(x)[4],(x)[5]
#endif


char *server_table[][2] = {
	{ "", "" },			/* 0: Unfiltered */
	{ "208.67.222.222", "" },	/* 1: OpenDNS */
	{ "", "" },			/* 2: Discontinued Norton Connect Safe */
	{ "", "" },			/* 3: Discontinued Norton Connect Safe */
	{ "", "" },			/* 4: Discontinued Norton Connect Safe */
	{ "77.88.8.88", "" },		/* 5: Secure Mode safe.dns.yandex.ru */
	{ "77.88.8.7", "" },		/* 6: Family Mode family.dns.yandex.ru */
	{ "208.67.222.123", "" },	/* 7: OpenDNS Family Shield */
	{ "", "" },			/* 8: Custom1 */
	{ "", "" },			/* 9: Custom2 */
	{ "", "" },			/* 10: Custom3 */
	{ "", "" },			/* 11: Router */
	{ "8.26.56.26", "" },		/* 12: Comodo Secure DNS */
	{ "9.9.9.9", "" },		/* 13: Quad9 */
	{ "185.228.168.9", "" },	/* 14: CleanBrowsing Security */
	{ "185.228.168.10", "" },	/* 15: CleanBrowsing Adult */
	{ "185.228.168.168", "" },	/* 16: CleanBrowsing Family */
	{ "1.1.1.2", "" },	/* 17: Cloudflare Secure Mode */
	{ "1.1.1.3", "" },	/* 18: Cloudflare Secure and Family */
	{ "94.140.14.14", "" },	/* 19: ADGUARD AD block mode*/
	{ "94.140.14.15", "" },	/* 20: ADGUARD AD block and Family mode*/
	{ NULL, NULL}	/* End of the table*/
};

#ifdef RTCONFIG_IPV6
char *server6_table[][2] = {
	{"", ""},		/* 0: Unfiltered */
	{"", ""},		/* 1: OpenDNS */
	{"", ""},		/* 2: Discontinued Norton Connect Safe */
	{"", ""},		/* 3: Discontinued Norton Connect Safe */
	{"", ""},		/* 4: Discontinued Norton Connect Safe */
	{"2a02:6b8::feed:bad","2a02:6b8:0:1::feed:bad"},		/* 5: Secure Mode safe.dns.yandex.ru */
	{"2a02:6b8::feed:a11","2a02:6b8:0:1::feed:a11"},		/* 6: Family Mode family.dns.yandex.ru */
	{"", ""},		/* 7: OpenDNS Family Shield */
	{"", ""},		/* 8: Custom1 - not supported yet */
	{"", ""},		/* 9: Custom2 - not supported yet */
	{"", ""},		/* 10: Custom3 - not supported yet */
	{"", ""},		/* 11: Router  - semi-supported, refer dnsfilter_setup_dnsmasq() */
	{"", ""},		/* 12: Comodo Secure DNS */
	{"2620:fe::fe", "2620:fe::9"},	/* 13: Quad9 */
	{"2a0d:2a00:1::2", "2a0d:2a00:2::2"},	/* 14: CleanBrowsing Security */
	{"2a0d:2a00:1::1", "2a0d:2a00:2::1"},	/* 15: CleanBrowsing Adult */
	{"2a0d:2a00:1::", "2a0d:2a00:2::"},	/* 16: CleanBrowsing Family */
	{"2606:4700:4700::1112", "2606:4700:4700::1002" },	/* 17: Cloudflare Secure Mode */
	{"2606:4700:4700::1113", "2606:4700:4700::1003" },	/* 18: Cloudflare Secure and Family */
	{"2a10:50c0::ad1:ff", "2a10:50c0::ad2:ff" },	/* 19: ADGUARD AD block mode*/
	{"2a10:50c0::bad1:ff", "2a10:50c0::bad2:ff" },	/* 20: ADGUARD AD block and Family mode*/
	{ NULL, NULL}	/* End of the table*/
};
#endif

extern char *read_whole_file(const char *target){
	FILE *fp;
	char *buffer, *new_str;
	int i;
	unsigned int read_bytes = 0;
	unsigned int each_size = 1024;

	if((fp = fopen(target, "r")) == NULL)
		return NULL;

	buffer = (char *)malloc(sizeof(char)*each_size);
	if(buffer == NULL){
		_dprintf("No memory \"buffer\".\n");
		fclose(fp);
		return NULL;
	}
	memset(buffer, 0, each_size);

	while ((i = fread(buffer+read_bytes, each_size * sizeof(char), 1, fp)) == 1){
		read_bytes += each_size;
		new_str = (char *)malloc(sizeof(char)*(each_size+read_bytes));
		if(new_str == NULL){
			_dprintf("No memory \"new_str\".\n");
			free(buffer);
			fclose(fp);
			return NULL;
		}
		memset(new_str, 0, sizeof(char)*(each_size+read_bytes));
		memcpy(new_str, buffer, read_bytes);

		free(buffer);
		buffer = new_str;
	}

	fclose(fp);
	return buffer;
}

extern char *get_line_from_buffer(const char *buf, char *line, const int line_size){
	int buf_len, len;
	char *ptr;

	if(buf == NULL || (buf_len = strlen(buf)) <= 0)
		return NULL;

	if((ptr = strchr(buf, '\n')) == NULL)
		ptr = (char *)(buf+buf_len);

	if((len = ptr-buf) < 0)
		len = buf-ptr;
	++len; // include '\n'.

	memset(line, 0, line_size);
	strncpy(line, buf, len);

	return line;
}

#ifdef HND_ROUTER
// defined (__GLIBC__) && !defined(__UCLIBC__)
#if 0
size_t strlcpy(char *dst, const char *src, size_t size)
{
	size_t srclen, len;

	srclen = strlen(src);
	if (size <= 0)
		return srclen;

	len = (srclen < size) ? srclen : size - 1;
	memcpy(dst, src, len); /* should not overlap */
	dst[len] = '\0';

	return srclen;
}

size_t strlcat(char *dst, const char *src, size_t size)
{
	size_t dstlen;
	char *null;

	null = memchr(dst, '\0', size);
	if (null == NULL)
		null = dst + size;
	dstlen = null - dst;

	return dstlen + strlcpy(null, src, size - dstlen);
}
#else /*Replace strlcpy and strlcat by the original version implemented by BSD*/
/* Implementation of strlcpy() for platforms that don't already have it. */
/*
 * Copy src to string dst of size siz.  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz == 0).
 * Returns strlen(src); if retval >= siz, truncation occurred.
 */
size_t
strlcpy(char *dst, const char *src, size_t siz)
{
	char *d = dst;
	const char *s = src;
	size_t n = siz;
	if(!dst || !src)
		return 0;
	/* Copy as many bytes as will fit */
	if (n != 0) {
		while (--n != 0) {
			if ((*d++ = *s++) == '\0')
				break;
		}
       }
	/* Not enough room in dst, add NUL and traverse rest of src */
	if (n == 0) {
		if (siz != 0)
			*d = '\0';		/* NUL-terminate dst */
		while (*s++)
			;
	}
	return(s - src - 1);	/* count does not include NUL */
}

/*
 * Appends src to string dst of size siz (unlike strncat, siz is the
 * full size of dst, not space left).  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz <= strlen(dst)).
 * Returns strlen(src) + MIN(siz, strlen(initial dst)).
 * If retval >= siz, truncation occurred.
 */
size_t
strlcat(char *dst, const char *src, size_t siz)
{
	char *d = dst;
	const char *s = src;
	size_t n = siz;
	size_t dlen;
	if(!dst || !src)
		return 0;
	/* Find the end of dst and adjust bytes left but don't go past end */
	while (n-- != 0 && *d != '\0')
		d++;
	dlen = d - dst;
	n = siz - dlen;
	if (n == 0)
		return(dlen + strlen(s));
	while (*s != '\0') {
		if (n != 1) {
			*d++ = *s;
			n--;
		}
		s++;
	}
	*d = '\0';
	return(dlen + (s - src));	/* count does not include NUL */
}

#endif
#endif

in_addr_t inet_addr_(const char *addr)
{
	struct in_addr in;
	return inet_aton(addr, &in) ? in.s_addr : INADDR_ANY;
}

int inet_equal(const char *addr1, const char *mask1, const char *addr2, const char *mask2)
{
	return ((inet_network(addr1) & inet_network(mask1)) ==
		(inet_network(addr2) & inet_network(mask2)));
}

int inet_intersect(const char *addr1, const char *mask1, const char *addr2, const char *mask2)
{
	in_addr_t max_netmask = inet_network(mask1) | inet_network(mask2);
	return ((inet_network(addr1) & max_netmask) ==
		(inet_network(addr2) & max_netmask));
}

int inet_deconflict(const char *addr1, const char *mask1, const char *addr2, const char *mask2, struct in_addr *result)
{
	const static struct class {
		in_addr_t network;
		in_addr_t netmask;
	} classes[] = {
		{ 0xc0a80000, 0xffff0000 }, /* 192.168.0.0/16 */
		{ 0xac100000, 0xfff00000 }, /* 172.16.0.0/12 */
		{ 0x0a000000, 0xff000000 }, /* 10.0.0.0/8 */
		{ 0, 0 }
	};
	struct class *class, *found;
	in_addr_t lan_ipaddr = inet_network(addr1);
	in_addr_t lan_netmask = inet_network(mask1);
	in_addr_t wan_ipaddr = inet_network(addr2);
	in_addr_t wan_netmask = inet_network(mask2);
	in_addr_t max_netmask = lan_netmask | wan_netmask;

	if ((lan_ipaddr & max_netmask) != (wan_ipaddr & max_netmask)) {
		/* not really intersecting */
		return 0;
	}

	found = NULL;
	for (class = (struct class *) classes; class->network; class++) {
		if (~lan_netmask > ~class->netmask)
			continue;
		if ((lan_ipaddr & class->netmask) != class->network)
			found = found ? : class;
		else if (~lan_netmask < ~class->netmask) {
			found = class;
			lan_ipaddr += ~lan_netmask + 1;
			break;
		}
	}
	if (found)
		lan_ipaddr = found->network | (lan_ipaddr & ~found->netmask);
	else {
		/* non-private address or too big network mask,
		 * address might become non-RFC1918 */
		lan_ipaddr += ~lan_netmask + 1;
	}

	if (result)
		result->s_addr = htonl(lan_ipaddr);

	return found ? 1 : 2;
}

/**
 * Validate an IPv4 address
 * @addr:	pointer to IPv4 address.
 * 	0:	Legal IPv4 address
 *  otherwise:	illegal IPv4 address
 */
int illegal_ipv4_address(char *addr)
{
	int l, r, v[4];
	char *p, *q;

	if (!addr)
		return -1;

	/* all characteristics valid? */
	for (p = addr; *p != '\0'; ++p) {
		if (isdigit(*p) || *p == '.')
			continue;

		return -2;
	}

	/* check length of addr */
	l = strlen(addr);
	if (l < 7 || l > 15)
		return -3;

	/* check value of addr */
	if ((r = sscanf(addr, "%d.%d.%d.%d", &v[0], &v[1], &v[2], &v[3])) != 4)
		return -4;

	if (v[0] < 0 || v[0] > 255 || v[1] < 0 || v[1] > 255 ||
	    v[2] < 0 || v[2] > 255 || v[3] < 0 || v[3] > 255)
		return -6;

	/* exclude 0.0.0.0 */
	if (!v[0] && !v[1] && !v[2] && !v[3])
		return -7;

	/* leading '0'? */
	for (p = q = addr - 1; p != NULL; p = q) {
		q = strchr(p + 1, '.');
		if (*(p + 1) == '0' && safe_atoi(p + 1) != 0)
			return -8;
	}

	return 0;
}

/**
 * Validate an IPv4 address
 * @addr:	pointer to IPv4 address.
 * 	0:	Legal IPv4 address
 *  otherwise:	illegal IPv4 address
 */
int illegal_ipv4_netmask(char *netmask)
{
	int n, r, v[4], flag;
	char *p, *q;

	if (!netmask)
		return -1;

	/* all characteristics valid? */
	for (p = netmask; *p != '\0'; ++p) {
		if (isdigit(*p) || *p == '.')
			continue;

		return -2;
	}

	/* CIDR? */
	if (!strchr(netmask, '.')) {
		n = safe_atoi(netmask);
		if (n < 0 || n >= 32)
			return -3;

		if (*netmask == '0')
			return -4;

		return 0;
	}

	if ((r = sscanf(netmask, "%d.%d.%d.%d", &v[0], &v[1], &v[2], &v[3])) != 4)
		return -5;

	if (v[0] < 0 || v[0] > 255 || v[1] < 0 || v[1] > 255 ||
	    v[2] < 0 || v[2] > 255 || v[3] < 0 || v[3] > 255)
		return -6;

	n = (v[0] << 24) | (v[1] << 16) | (v[2] << 8) | v[3];
	for (flag = 0; n > 0; n >>= 1) {
		if (n & 1) {
			flag = 1;
			continue;
		}

		if (flag) {
			dbg("%s: netmask [%s ==> %x.%x.%x.%x] has non-continuous zero bit.\n",
				__func__, netmask, v[0], v[1], v[2], v[3]);
			return -7;
		}
	}

	/* leading '0'? */
	for (p = q = netmask - 1; p != NULL; p = q) {
		q = strchr(p + 1, '.');
		if (*(p + 1) == '0' && safe_atoi(p + 1) != 0)
			return -8;
	}

	return 0;
}

#if defined(RTCONFIG_QCA) || defined(RTCONFIG_LANTIQ)
void convert_mac_string(char *mac)
{
	int i;
	char mac_str[18], mac_str_t[18];
	memset(mac_str,0,sizeof(mac_str));

	for(i=0;i<strlen(mac);i++)
	{
		if(*(mac+i)>0x60 && *(mac+i)<0x67){
			snprintf(mac_str_t, sizeof(mac_str), "%s%c",mac_str,*(mac+i)-0x20);
			strlcpy(mac_str, mac_str_t, sizeof(mac_str));
		}
		else{
			snprintf(mac_str_t, sizeof(mac_str), "%s%c",mac_str,*(mac+i));
			strlcpy(mac_str, mac_str_t, sizeof(mac_str));
		}

	}
	strlcpy(mac, mac_str, strlen(mac_str) + 1);
}
#endif

#if defined(RTCONFIG_PORT_BASED_VLAN) || defined(RTCONFIG_TAGGED_BASED_VLAN)
/**
 * Get all vlan rules.
 * @return:	pointer to vlan_rules_s structure.
 *  NULL:	vlan_rule_list nvram variable is not set or allocate memory error.
 *  otherwise:	Success, even none of any legal vlan rule exist.
 * 		Caller is incharge of release returned pointer!
 */
struct vlan_rules_s *get_vlan_rules(void)
{
	int br_index = VLAN_START_IFNAMES_INDEX;
	char *nv, *nvp, *b;
	struct vlan_rules_s *vlan_rules;
	struct vlan_rule_s *r;
#if defined(RTCONFIG_PORT_BASED_VLAN)
	char *enable, *desc, *portset, *wlmap, *subnet_name, *intranet;
#elif defined(RTCONFIG_TAGGED_BASED_VLAN)
	char *enable, *vid, *prio, *wanportset, *lanportset, *wl2gset;
	char *wl5gset, *subnet_name, *internet, *public_vlan;
#endif

	nv = nvp = strdup(nvram_safe_get("vlan_rulelist"));
	if (!nv)
		return NULL;

	vlan_rules = malloc(sizeof(struct vlan_rules_s));
	if (!vlan_rules)
		return NULL;

	r = &vlan_rules->rules[0];
	memset(vlan_rules, 0, sizeof(struct vlan_rules_s));
	while ((b = strsep(&nvp, "<")) != NULL && vlan_rules->nr_rules < ARRAY_SIZE(vlan_rules->rules)) {
#if defined(RTCONFIG_PORT_BASED_VLAN)
		if ((vstrsep(b, ">", &enable, &desc, &portset, &wlmap, &subnet_name, &intranet) != 6))
			continue;

		r->enable = safe_atoi(enable);
		strlcpy(r->desc, desc, sizeof(r->desc));
		strlcpy(r->portset, portset, sizeof(r->portset));
		strlcpy(r->wlmap, wlmap, sizeof(r->wlmap));
		strlcpy(r->subnet_name, subnet_name, sizeof(r->subnet_name));
		strlcpy(r->intranet, intranet, sizeof(r->intranet));
#elif defined(RTCONFIG_TAGGED_BASED_VLAN)
		if ((vstrsep(b, ">", &enable, &vid, &prio, &wanportset,
		     &lanportset, &wl2gset, &wl5gset, &subnet_name,
		     &internet, &public_vlan ) != 10))
			continue;

		r->enable = safe_atoi(enable);
		strlcpy(r->vid, vid, sizeof(r->vid));
		strlcpy(r->prio, prio, sizeof(r->prio));
		strlcpy(r->wanportset, wanportset, sizeof(r->wanportset));
		strlcpy(r->lanportset, lanportset, sizeof(r->lanportset));
		strlcpy(r->wl2gset, wl2gset, sizeof(r->wl2gset));
		strlcpy(r->wl5gset, wl5gset, sizeof(r->wl5gset));
		strlcpy(r->subnet_name, subnet_name, sizeof(r->subnet_name));
		strlcpy(r->internet, internet, sizeof(r->internet));
		strlcpy(r->public_vlan, public_vlan, sizeof(r->public_vlan));
#endif
		if (r->enable) {
			if (safe_atoi(r->vid) == 1 || !strcmp(r->subnet_name, "default"))
				strlcpy(r->br_if, nvram_get("lan_ifname")? : nvram_default_get("lan_ifname"), sizeof(r->br_if));
			else
				snprintf(r->br_if, sizeof(r->br_if), "br%d", br_index++);
		}

		vlan_rules->nr_rules++;
		r++;
	}
	free(nv);

	return vlan_rules;
}
#endif

/**
 * Executed at start of config_switch()
 */
void pre_config_switch(void)
{
	if (__pre_config_switch)
		__pre_config_switch();
}

/**
 * Executed at end of config_switch()
 */
void post_config_switch(void)
{
	if (__post_config_switch)
		__post_config_switch();
}

/**
 * Executed at end of start_lan()
 */
void post_start_lan(void)
{
	if (__post_start_lan)
		__post_start_lan();
}

/**
 * Executed at end of start_lan_wl()
 */
void post_start_lan_wl(void)
{
	if (__post_start_lan_wl)
		__post_start_lan_wl();
}

/**
 * Find-out whether a model use software bridge to implement IPTV or not.
 * If yes, implement and return true in model-specific __sw_based_iptv().
 * set_basic_ifname_vars() will use bridge interface, e.g., brvXXX,
 * as WAN interface instead of VLAN interface, e.g., vlanXXX.
 * @return:
 * 	0:	This model doesn't use software bridge to implement IPTV.
 *  otherwise:	This model do use software bridge to implement IPTV.
 */
int sw_based_iptv(void)
{
	if (__sw_based_iptv)
		return __sw_based_iptv();
	return 0;
}

/**
 * If a model need to use custom VLAN to bridge WAN/STB port, define your
 * own __sw_bridge_iptv_different_switches() and return 1 there.
 * @return:
 * 	0:	This model doesn't use VLAN to bridge WAN/STB port on different switches.
 *  otherwise:	This model do use VLAN to bridge WAN/STB port on different switches.
 */
int sw_bridge_iptv_different_switches(void)
{
	if (__sw_bridge_iptv_different_switches)
		return __sw_bridge_iptv_different_switches();
	return 0;
}

/* Get VLAN ID that is used to bridge software-based IPTV implementation.
 * @return:	VLAN ID
 */
int get_sw_bridge_iptv_vid(void)
{
	int vid;

	vid = nvram_get_int("switch_br_vid");
	if (vid > 2 && vid < 4096)
		return vid;

	return SW_IPTV_BRIDGE_VID;
}

/* Write @val to /sys/class/net/@iface/@param.
 * @iface:	interface name
 * @path:	relative path and filename under /sys/class/net/@iface
 * @val:
 * @return:
 * 	0:	success
 *     -1:	invalid parameter
 *     -2:	@val is not writen completely.
 */
int set_netdev_sysfs_param(const char *iface, const char *param, const char *val)
{
	int r;
	char path[sizeof(SYS_CLASS_NET) + 1 + IFNAMSIZ + 1 + 256];

	if (!iface || !param || !val)
		return -1;

	snprintf(path, sizeof(path), "%s/%s/%s", SYS_CLASS_NET, iface, param);
	r = f_write_string(path, val, 0, 0);

	return (r < strlen(val))? -2 : 0;
}

#if defined(RTCONFIG_COOVACHILLI) || \
    defined(RTCONFIG_PORT_BASED_VLAN) || defined(RTCONFIG_TAGGED_BASED_VLAN)
/* Don't leave { 0,0 } at end */
static const struct ip_mask_s private_network_tbl[] = {
	{ IP2UINT(192,168,0,0),	16 },
	{ IP2UINT(172,16,0,0),	12 },
	{ IP2UINT(10,0,0,0),	8 },
};

/**
 * Get netmask of specified @cidr
 * @cidr:	CIDR
 * @return:	netmask of specified @cidr.
 * NOTE:	This function doesn't validate input parameter, @cidr.
 */
static inline uint32_t get_netmask(int cidr)
{
	int host_bits;
	uint32_t m;

	host_bits = 32 - cidr;
	m = (((~0U) >> host_bits) << host_bits);

	return m;
}

/**
 * Test specified network in one class specified by @pt.
 * @pt:		Scan target class. Pointer to ip_mask_s structure.
 * 		@exp_ip/@exp_cidr MUST BELONGS TO THIS CLASS!
 * @pk_tbl:	Known networks. Pointer to ip_mask_s structure array.
 * 		ALL ELEMENTS OF LAST ITEM MUST BE ZERO!
 * @exp_ip:	Pointer to expected IP integer, host-endian.
 * 		Highest bytes is first number in IP, Second one is 2-nd number in IP, etc.
 * @exp_cidr:	Expected cidr integer.
 * @return:
 *     >0:	Specified network conflicts with known network in the class and new
 *     		network is suggested in @exp_ip/@exp_cidr.
 * 	0:	Specified network is not conflicts with any known network in the class.
 *     -1:	Invalid parameter.
 *     -12:	@exp_ip/@exp_cidr is not belongs to scan target class.
 *     -13:	@exp_ip/@exp_cidr is larger than specified class.
 *     -14:	One or more known networks is larger than specified class.
 *     -15:	Can't find available network in specified class.
 *     <0:	Another error.
 */
static int test_one_class(const struct ip_mask_s *pt, const struct ip_mask_s *pk_tbl, uint32_t *exp_ip, uint32_t exp_cidr)
{
	int i, c, found, s_cidr = 30, max_cidr, ccnt = 0;
	uint32_t start_net, s, next_s, d, delta, class_mask, m, mask, new_ip = 0;
	const struct ip_mask_s *p;

	if (!pt || !pt->ip || !pt->cidr || pt->cidr > 30 || !pk_tbl ||
	    !exp_ip || exp_cidr <= 0 || exp_cidr > 30)
		return -1;

	/* Is exp_ip/exp_cidr belongs this private IP network? */
	mask = get_netmask(pt->cidr);
	if ((*exp_ip & mask) != (pt->ip & mask)) {
		dbg("%08x/%d is not belongs to %08x/%d, mask %08x\n",
			*exp_ip, exp_cidr, pt->ip, pt->cidr, mask);
		return -12;
	}

	if (exp_cidr < pt->cidr)
		return -13;

	for (p = pk_tbl; p->ip && p->cidr; ++p) {
		/* Ignore wrong cidr.
		 * For example, cidr of 192.168.x.x is lower than or equal to 16.
		 */
		if (p->cidr <= pt->cidr) {
			dbg("Ignore %08x/%d due to cidr lower than %08x/%d\n",
				p->ip, p->cidr, pt->ip, pt->cidr);
			continue;
		}

		/* Skip different class. */
		mask = get_netmask(pt->cidr);
		if ((p->ip & mask) != (pt->ip & mask))
			continue;

		if (p->cidr < pt->cidr)
			return -14;
	}

	/* Find first not conflicts network. */
	class_mask = get_netmask(pt->cidr);
	for (i = min(s_cidr, exp_cidr), found = 0; !found && i >= pt->cidr; i--) {
		m = mask = get_netmask(i);
		delta = ~mask + 1;
		s = start_net = *exp_ip & mask;
		dbg("Exp %08x/%d, test %08x/%d, delta 0x%x mask 0x%x, loop %d\n",
			*exp_ip, exp_cidr, s, i, delta, mask, 1U << (i - pt->cidr));
		next_s = 0;
		while (!found && ((next_s & mask) != (start_net & mask))) {
			// dbg("\t%08x/%08x,%08x\n", s, mask, m);
			for (c = 0, p = pk_tbl, max_cidr = i; p->ip && p->cidr; ++p) {
				m = get_netmask(min(i, p->cidr));
				if ((s & m) == (p->ip & m)) {
					if (p->cidr < max_cidr)
						max_cidr = p->cidr;
					if (++ccnt < 10) {
						dbg("\t%08x/%d conflicts, max_cidr %d.\n", p->ip, p->cidr, max_cidr);
						logmessage("", "\t%08x/%d conflicts, max_cidr %d.\n", p->ip, p->cidr, max_cidr);
					}
					c++;
				}
			}

			if (c == 0) {
				new_ip = s;
				found = 1;
				break;
			}
			m = get_netmask(max_cidr);
			d = ~m + 1;
			s = (s + d) & m;
			if ((s & class_mask) != (pt->ip & class_mask))
				s = pt->ip & class_mask;
#if 0
			if (m != mask) {
				dbg("\tnext %08x/%d, max_cidr %d, d %x delta %x m %x mask %x\n", s, i, max_cidr, d, delta, m, mask);
			}
#endif
			next_s = s;
		}
	}

	if (found) {
		m = get_netmask(exp_cidr);
		if ((new_ip & m) == (*exp_ip & m))
			return 0;
		else {
			dbg("Find non-conflicts network %08x/%d for %08x/%d\n",
				s, i, *exp_ip, exp_cidr);
			// dbg("New IP/mask %08x/%d\n", new_ip, exp_cidr);
			*exp_ip = new_ip;
			return 1;
		}
	}

	return -15;
}

/**
 * Append an character format IP/mask to an ip_mask_s structure to an array,
 * if IPv4 address and netmask is valid and array is not full.
 * @nr:		Number of available items in an array specified by @p.
 * @p:		Pointer to an ip_mask_s array.
 * @ip_cidr_str:IPv4 address and netmask in X.X.X.X/Y
 * @return:
 * 	0:	OK
 *     -1:	Invalid parameter.
 *     -2:	Out of array space.
 *     -3:	Netmask absent.
 *     -4:	Invalid IPv4 address or netmask.
 */
static int fill_char_ip_cidr_to_ip_mask_s(unsigned int *nr, struct ip_mask_s **p, char *ip_cidr_str)
{
	int r, ip[4], mask[4];
	uint32_t cidr, m;
	char ip_mask[sizeof("192.168.100.200/255.255.255.255XXX")], *pc;
	char ip_str[sizeof("192.168.100.200XXX")], cidr_str[sizeof("255.255.255.255XXX")];

	if (!nr || !p || !ip_cidr_str) {
		dbg("%s: invalid parameter. nr %p p %p ip_cidr_str %p\n", __func__, nr, p, ip_cidr_str);
		return -1;
	}
	if (*nr <= 0) {
		dbg("%s: Out of array. (ip_cidr %s)\n", __func__, ip_cidr_str);
		return -2;
	}

	strlcpy(ip_mask, ip_cidr_str, sizeof(ip_mask));
	if ((pc = strchr(ip_mask, '/')) == NULL) 
		return -3;

	strlcpy(ip_str, ip_mask, min(pc - ip_mask + 1, sizeof(ip_str)));
	strlcpy(cidr_str, pc + 1, sizeof(cidr_str));
	if (illegal_ipv4_address(ip_str) || illegal_ipv4_netmask(cidr_str))
		return -4;

	if ((r = sscanf(ip_str, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3])) != 4)
		return -5;

	if ((r = sscanf(cidr_str, "%d.%d.%d.%d", &mask[0], &mask[1], &mask[2], &mask[3])) == 4) {
		/* convert x.x.x.x netmask to cidr. */
		m = ((mask[0] & 0xFF) << 24) | ((mask[1] & 0xFF) << 16) |
		    ((mask[2] & 0xFF) & 8) | (mask[3] & 0xFF);
		cidr = 0;
		while (m > 0) {
			cidr++;
			m <<= 1;
		}
	} else {
		cidr = safe_atoi(cidr_str);
	}

	(*p)->ip = ((ip[0] & 0xFF) << 24) | ((ip[1] & 0xFF) << 16) |
	        ((ip[2] & 0xFF) << 8) | (ip[3] & 0xFF);
	(*p)->cidr = cidr;
	(*nr)--;
	(*p)++;

	return 0;
}

/**
 * Append an character format IP/mask to an ip_mask_s structure to an array,
 * if IPv4 address and netmask is valid and array is not full.
 * @nr:		Number of available items in an array specified by @p.
 * @p:		Pointer to an ip_mask_s array.
 * @ip_str:	IPv4 address
 * @mask_str:	IPv4 netmask in X.X.X.X or CIDR format.
 * @return:
 * 	0:	OK
 *     -1:	Invalid parameter.
 *     -2:	Out of array space.
 *     -3:	Invalid IPv4 address or netmask.
 *     -4:	Invalid IPv4 address format
 *     -5:	First byte of IPv4 address is zero.
 *     -6:	Mask is zero or greater than 32.
 */
static int fill_char_ip_mask_to_ip_mask_s(unsigned int *nr, struct ip_mask_s **p, char *ip_str, char *mask_str)
{
	int r, ip[4], mask[4];
	uint32_t cidr, m;

	if (!nr || !p || !ip_str || !mask_str) {
		dbg("%s: invalid parameter. nr %p p %p ip_str %p mask_str %p\n", __func__, nr, p, ip_str, mask_str);
		return -1;
	}
	if (*nr <= 0) {
		dbg("%s: Out of array. (ip/cidr %s/%s)\n", __func__, ip_str, mask_str);
		return -2;
	}

	if (illegal_ipv4_address(ip_str) || illegal_ipv4_netmask(mask_str))
		return -3;

	if ((r = sscanf(ip_str, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3])) != 4)
		return -4;

	if (!ip[0])
		return -5;

	if ((r = sscanf(mask_str, "%d.%d.%d.%d", &mask[0], &mask[1], &mask[2], &mask[3])) == 4) {
		/* convert x.x.x.x netmask to cidr. */
		m = ((mask[0] & 0xFF) << 24) | ((mask[1] & 0xFF) << 16) |
		    ((mask[2] & 0xFF) << 8) | (mask[3] & 0xFF);
		cidr = 0;
		while (m > 0) {
			cidr++;
			m <<= 1;
		}
	} else {
		cidr = safe_atoi(mask_str);
	}

	if (!cidr || cidr > 32)
		return -6;

	(*p)->ip = ((ip[0] & 0xFF) << 24) | ((ip[1] & 0xFF) << 16) |
	        ((ip[2] & 0xFF) << 8) | (ip[3] & 0xFF);
	(*p)->cidr = cidr;
	(*nr)--;
	(*p)++;

	return 0;
}

#if defined(RTCONFIG_IPSEC)
/**
 * Helper function to fill known IPSec server networks.
 * @nr:
 * @p:
 * @excl:	Bitmask combination of EXCLUDE_NET_XXX.
 * @return:
 * 	1:	Don't add IPSec server networks to list due to excl is ipsec_profile_
 * 	0:	success
 *    < 0:	Number of fail added networks.
 */
static int _get_ipsec_server_networks(unsigned int *nr, struct ip_mask_s **p, uint32_t excl)
{
	int i, j, fail = 0, r;
	char *nv, *nvp, *now, *next;
	char n[sizeof("ipsec_profile_1XXX")], *a[38];

	if (!(excl & EXCLUDE_NET_IPSEC_SERVER))
		return 1;

	/* Exclude IPSec VPN Server, ipsec_profile_1~5
	 * Host to net example:
	 * 4>test1>null>null>wan>>1>abcdekeyword>null>null>null>null>null>1>10.10.10.0/24>null>1>null>null>0>null>null>null>0>>>eap-md5>1>500>4500>10>1>null>null>null>null><10.10.10.1<10.10.10.2<10.10.10.3<10.10.10.4>0
	 * 	0:	type
	 * 	14:	Virtual IP
	 * Net to net example:
	 * 1>test2>null>null>wan>>1>QQKKABCD><10.100.200.0/24<10.100.201.0/24<10.100.202.0/24>0><10.200.100.0/24<10.200.101.0/24<10.200.102.0/24>0>tunnel>null>null>null>1>auto>auto>0>lident>ridenty>172800>0>null>null>eap-md5>1>500>4500>10>1>auto>auto>3600>3>null>0
	 * 	0:	type
	 *	8:	Local private subnets, '<' is seperate char if two or more subnet exist.
	 *	10:	Remote private subnets, '<' is seperate char if two or more subnet exist.
	 */
	for (i = 1; i <= 5; ++i) {
		snprintf(n, sizeof(n), "ipsec_profile_%d", i);

		nv = nvp = strdup(nvram_safe_get(n));
		if (!nv)
			continue;
		if (*nv == '\0') {
			free(nv);
			continue;
		}

		if ((r = vstrsep(nv, ">", a, a + 1, a + 2, a + 3, a + 4,
		    a + 5, a + 6, a + 7, a + 8, a + 9, a + 10,
		    a + 11, a + 12, a + 13, a + 14, a + 15, a + 16,
		    a + 17, a + 18, a + 19, a + 20, a + 21, a + 22,
		    a + 23, a + 24, a + 25, a + 26, a + 27, a + 28,
		    a + 29, a + 30, a + 31, a + 32, a + 33, a + 34,
		    a + 35, a + 36, a + 37)) != 38) {
			dbg("%s: Parse IPSec server profile%d [%s] fail, r %d\n", __func__, i, nv, r);
			free(nv);
			continue;
		}

		switch (safe_atoi(a[0])) {
		case 4:
			/* Host to net, Virtual IP is saved at a[14] */
			r = fill_char_ip_cidr_to_ip_mask_s(nr, p, a[14]);
			if (r < 0) {
				dbg("%s: Add %s of %s failed, return %d\n", __func__, a[14], n, r);
				fail++;
			}
			break;
		case 1:
			/* Net to net,
			 * local private subnets are saved at a[8],
			 * remote private subnets are saved at a[11]
			 */
			for (j = 0; j < 2; ++j) {
				now = next = (j == 0)? a[8] : a[10];
				while ((now = strsep(&next, "<")) != NULL) {
					if (*now == '\0')
						continue;
					r = fill_char_ip_cidr_to_ip_mask_s(nr, p, now);
					if (r < 0) {
						dbg("%s: Add %s of %s failed, return %d\n", __func__, now, n, r);
						fail++;
					}
				}
			}
			break;
		default:
			dbg("%s: Unknown IPSec profile type %s\n", __func__, a[0]? : "<NULL>");
			break;
		}
		free(nv);
	}

	return -fail;
}

/**
 * Helper function to fill known IPSec client networks.
 * @nr:
 * @p:
 * @excl:	Bitmask combination of EXCLUDE_NET_XXX.
 * @return:
 * 	1:	Don't add IPSec client networks to list due to excl is ipsec_profile_client_
 * 	0:	success
 *    < 0:	Number of fail added networks.
 */
static int _get_ipsec_client_networks(unsigned int *nr, struct ip_mask_s **p, uint32_t excl)
{
	int i, j, fail = 0, r;
	char *nv, *nvp, *now, *next;
	char n[sizeof("ipsec_profile_client_1XXX")], *a[38];

	if (!(excl & EXCLUDE_NET_IPSEC_CLIENT))
		return 1;

	/* Exclude IPSec VPN Client, ipsec_profile_client_1~5
	 * example:
	 * ipsec_profile_client_1=2>vpn_profile_1_c1>0>192.168.100.1>wan>192.168.1.90>1>ABCDEKEY><10.20.30.0/24>0><10.20.40.0/24>0>tunnel>null>null>null>1>auto>auto>0>LIDENTY>RIDENTY>172800>0>>>eap-md5>1>500>4500>10>1>auto>auto>3600>3>null>0
	 * ipsec_profile_client_2=2>vpn_profile_2_c2>0>192.168.200.1>wan2>10.2.2.168>1>QWERTYUI><10.30.0.0/24>0><10.40.0.0/24>0>tunnel>null>null>null>1>auto>auto>0>l2identy>r2identy>172800>0>>>eap-md5>1>500>4500>10>1>auto>auto>3600>3>null>0
	 * 	3:	gateway
	 *	5:	WAN IP, skip
	 *	8:	local private subnet
	 *	10:	remote private subnet
	 */
	for (i = 1; i <= 5; ++i) {
		snprintf(n, sizeof(n), "ipsec_profile_client_%d", i);

		nv = nvp = strdup(nvram_safe_get(n));
		if (!nv)
			continue;
		if (*nv == '\0') {
			free(nv);
			continue;
		}

		if ((r = vstrsep(nv, ">", a, a + 1, a + 2, a + 3, a + 4,
		    a + 5, a + 6, a + 7, a + 8, a + 9, a + 10,
		    a + 11, a + 12, a + 13, a + 14, a + 15, a + 16,
		    a + 17, a + 18, a + 19, a + 20, a + 21, a + 22,
		    a + 23, a + 24, a + 25, a + 26, a + 27, a + 28,
		    a + 29, a + 30, a + 31, a + 32, a + 33, a + 34,
		    a + 35, a + 36, a + 37)) != 38) {
			dbg("%s: Parse IPSec client profile%d [%s] fail, r %d\n", __func__, i, nv, r);
			free(nv);
			continue;
		}

		for (j = 0; j < 2; ++j) {
			now = next = (j == 0)? a[8] : a[10];
			while ((now = strsep(&next, "<")) != NULL) {
				if (*now == '\0')
					continue;
				r = fill_char_ip_cidr_to_ip_mask_s(nr, p, now);
				if (r < 0) {
					dbg("%s: Add %s of %s failed, return %d\n", __func__, now, n, r);
					fail++;
				}
			}
		}
		free(nv);
	}

	return -fail;
}
#else
static inline int _get_ipsec_server_networks(unsigned int *nr, struct ip_mask_s **p, uint32_t excl) { return 0; }
static inline int _get_ipsec_client_networks(unsigned int *nr, struct ip_mask_s **p, uint32_t excl) { return 0; }
#endif

/**
 * Get known networks used by another feature or some USB modem.
 * @nr:		Number of elements of @tbl array.
 * @tbl:	Pointer to ip_mask_s structure array.
 * @excl:	Bitmask combination of EXCLUDE_NET_XXX.
 * 		If not zero, exclude network of specified feature(s).
 * @return:
 * 	>= 0:	Number of filled ip_mask_s structure.
 * 	< 0:	Error
 */
static int get_known_networks(unsigned int nr, struct ip_mask_s *tbl, uint32_t excl)
{
	int i, r;
	char prefix[20];
	unsigned nr_orig = nr;
	struct ip_mask_s *p = tbl, *q;

	if (nr <= 0 || !tbl)
		return -1;

	/* Exclude network used by USB modem. */
	if (!(excl & EXCLUDE_NET_USB_MODEM)) {
		/* FIXME: Exclude 192.168.0.x and 192.168.100.x if and only if
		 * USB modem that need one of these networks is connected to DUT.
		 */
	}

	/* Exclude LAN */
	if (!(excl & EXCLUDE_NET_LAN)) {
		fill_char_ip_mask_to_ip_mask_s(&nr, &p, nvram_get("lan_ipaddr"), nvram_get("lan_netmask"));
	}

	/* Exclude all Static/DHCP WAN */
	if (!(excl & EXCLUDE_NET_WAN)) {
		for (i = WAN_UNIT_FIRST; i < WAN_UNIT_MAX; ++i) {
			snprintf(prefix, sizeof(prefix), "wan%d_", i);
			fill_char_ip_mask_to_ip_mask_s(&nr, &p, nvram_pf_get(prefix, "ipaddr"), nvram_pf_get(prefix, "netmask"));
		}
	}

	/* Exclude PPTP VPN Server */
	if (!(excl & EXCLUDE_NET_PPTP_SERVER)) {
		char ip_range[sizeof("192.168.100.200-255XXX")], *c;	/* e.g. 192.168.10.2.11 */

		strlcpy(ip_range, nvram_safe_get("pptpd_clients"), sizeof(ip_range));
		c = strchr(ip_range, '-');
		if (c != NULL)
			*c = '\0';
		r = fill_char_ip_mask_to_ip_mask_s(&nr, &p, ip_range, "24");
		if (r < 0) {
			dbg("%s: Add PPTP VPN Server %s to known networks list failed, return %d\n",
				__func__, ip_range, r);
		}
	}

#if defined(RTCONFIG_OPENVPN)
	/* Exclude OpenVPN Server */
	if (!(excl & EXCLUDE_NET_OPENVPN_SERVER)) {
		r = fill_char_ip_mask_to_ip_mask_s(&nr, &p, nvram_get("vpn_server_sn"), nvram_get("vpn_server_nm"));
		if (r < 0) {
			dbg("%s: Add OpenVPN Server %s/%s to known networks list failed, return %d\n",
				__func__, nvram_safe_get("vpn_server_sn"), nvram_safe_get("vpn_server_nm"), r);
		}
	}
#endif

	/* Exclude IPSec server/client */
	_get_ipsec_server_networks(&nr, &p, excl);
	_get_ipsec_client_networks(&nr, &p, excl);

#if defined(RTCONFIG_PORT_BASED_VLAN) || defined(RTCONFIG_TAGGED_BASED_VLAN)
	/* Exclude port/tagged based VLAN */
	if (!(excl & EXCLUDE_NET_VLAN)) {
		char *nv, *nvp, *b;
#if defined(RTCONFIG_PORT_BASED_VLAN)
		char *enable, *desc, *portset, *wlmap, *subnet_name, *intranet;
#elif defined(RTCONFIG_TAGGED_BASED_VLAN)
		char *enable, *vid, *prio, *wanportset, *lanportset, *wl2gset;
		char *wl5gset, *subnet_name, *internet, *public_vlan;
#endif

		nv = nvp = strdup(nvram_safe_get("vlan_rulelist"));
		while (nv != NULL && (b = strsep(&nvp, "<")) != NULL) {
#if defined(RTCONFIG_PORT_BASED_VLAN)
			if ((vstrsep(b, ">", &enable, &desc, &portset, &wlmap,
			     &subnet_name, &intranet) != 6))
#elif defined(RTCONFIG_TAGGED_BASED_VLAN)
			if ((vstrsep(b, ">", &enable, &vid, &prio, &wanportset,
			     &lanportset, &wl2gset, &wl5gset, &subnet_name,
			     &internet, &public_vlan ) != 10))
#endif
				continue;

			if (!strcmp(subnet_name, "default"))
				continue;
			r = fill_char_ip_cidr_to_ip_mask_s(&nr, &p, subnet_name);
			if (r < 0) {
				dbg("%s: Add VLAN %s to known networks list failed, return %d\n",
					__func__, subnet_name);
			}
		}
		if (nv != NULL)
			free(nv);
	}
#endif

#if defined(RTCONFIG_COOVACHILLI)
	/* Exclude Free Wi-Fi */
	if (!(excl & EXCLUDE_NET_FREE_WIFI)) {
		r = fill_char_ip_cidr_to_ip_mask_s(&nr, &p, nvram_get("chilli_net"));
		if (r < 0) {
			dbg("%s: Add Free Wi-Fi %s to known networks list failed, return %d\n",
				__func__, nvram_safe_get("chilli_net"));
		}
	}

	/* Exclude Captive Portal */
	if (!(excl & EXCLUDE_NET_CAPTIVE_PORTAL)) {
		r = fill_char_ip_cidr_to_ip_mask_s(&nr, &p, nvram_get("cp_net"));
		if (r < 0) {
			dbg("%s: Add Captive Portal %s to known networks list failed, return %d\n",
				__func__, nvram_safe_get("cp_net"));
		}
	}
#endif

	_dprintf("%s: %d known networks. excl %08x\n", __func__, nr_orig - nr, excl);
	for (q = tbl; q < p; ++q) {
		_dprintf("\t%08X/%d  (%d.%d.%d.%d/%d)\n", q->ip, q->cidr,
			(q->ip >> 24) & 0xFF, (q->ip >> 16) & 0xFF,
			(q->ip >> 8) & 0xFF, q->ip & 0xFF, q->cidr);
	}

	return nr_orig - nr;
}

/**
 * Test specified network with another known networks.
 * If conflicts with any known network, find alternate network in all scan target class.
 * @t_class:	Bitmasp of scan target classes.
 *  bit0:	Scan first class of private_network_tbl. (Class C private IP)
 *  bit1:	Scan second class of private_network_tbl. (Class B private IP)
 *  bit2:	Scan third class of private_network_tbl. (Class A private IP)
 * @exp_ip:	Pointer to expected IP integer, host-endian.
 * 		Highest bytes is first number in IP, Second one is 2-nd number in IP, etc.
 * @exp_cidr:	Expected cidr integer.
 * @excl:	Bitmask combination of EXCLUDE_NET_XXX.
 * 		If not zero, exclude network of specified feature(s).
 * @return:
 * 	 1:	Input @exp_ip/@exp_cidr is conflicts to at lease one known network and
 * 	 	new network is suggested in same parameters.
 *     		New cidr may smaller, larger network, than original value.  It's safe to use original cidr.
 * 	 0:	@exp_ip/@exp_cidr is not conflicts with any known networks.
 * 	-1:	Invalid parameter.
 *      <0:	Another error, come from test_one_class().
 */
int test_and_get_free_uint_network(int t_class, uint32_t *exp_ip, uint32_t exp_cidr, uint32_t excl)
{
	int i, exp_class_idx, r = -1, nr, may_conflicts = 0;
	uint32_t m, ip, cidr;
	const struct ip_mask_s *pv;
	struct ip_mask_s known_network_tbl[50], *kn;

	if (!exp_ip || exp_cidr <= 0 || exp_cidr > 30)
		return -1;

	if ((t_class >> ARRAY_SIZE(private_network_tbl)) > 0)
		return -1;

	/* Is exp_ip/exp_cidr belongs to one private IP network and will be scanned? */
	for (i = 0, exp_class_idx = -1, pv = private_network_tbl;
	     exp_class_idx < 0 && i < ARRAY_SIZE(private_network_tbl);
	     ++i, ++pv)
	{
		if (!((t_class >> i) & 1))
			continue;

		m = get_netmask(pv->cidr);
		if ((*exp_ip & m) != (pv->ip & m))
			continue;

		exp_class_idx = i;
	}

	/* TODO: Get known networks of another features. */
	nr = ARRAY_SIZE(known_network_tbl);
	memset(known_network_tbl, 0, sizeof(known_network_tbl));
	get_known_networks(--nr, known_network_tbl, excl);   /* leave last elements */

	for (i = 0, kn = &known_network_tbl[0];
	    i < ARRAY_SIZE(known_network_tbl) && kn->ip && kn->cidr;
	    ++i, ++kn)
	{
		m = get_netmask(min(exp_cidr, kn->cidr));
		if ((*exp_ip & m) == (kn->ip & m))
			may_conflicts++;
	}

	if (!may_conflicts)
		return 0;

	/* Try class own @exp_ip/@exp_cidr. */
	if (exp_class_idx >= 0 && exp_class_idx < ARRAY_SIZE(private_network_tbl)) {
		r = test_one_class(&private_network_tbl[exp_class_idx], known_network_tbl, exp_ip, exp_cidr);
		if (r >= 0)
			return r;
	}

	/* Try another scan target class. */
	for (i = 0, pv = private_network_tbl; i < ARRAY_SIZE(private_network_tbl); ++i, ++pv) {
		if (i == exp_class_idx || !((t_class >> i) & 1))
			continue;

		ip = pv->ip;
		cidr = 24;	/* Always start from class C level */
		dbg("Try %08x/%d for %08x/%d\n", ip, cidr, *exp_ip, exp_cidr);
		r = test_one_class(pv, known_network_tbl, &ip, cidr);
		if (r >= 0) {
			*exp_ip = ip;
			break;
		}
	}

	return r;
}

/**
 * Test specified network with another known networks.
 * If conflicts with any known network, find alternate network in all scan target class.
 * @t_class:	Bitmasp of scan target classes.
 *  bit0:	Scan first class of private_network_tbl. (Class C private IP)
 *  bit1:	Scan second class of private_network_tbl. (Class B private IP)
 *  bit2:	Scan third class of private_network_tbl. (Class A private IP)
 * @ip_cidr_str:Pointer to IP/mask string, at least 32 bytes.
 * 		If new network is suggested, @ip_cidr_str will have new value.
 * @exp_ip:	Pointer to expected IP integer, host-endian.
 * 		Highest bytes is first number in IP, Second one is 2-nd number in IP, etc.
 * @excl:	Bitmask combination of EXCLUDE_NET_XXX.
 * 		If not zero, exclude network of specified feature(s).
 * @return:
 * 	 1:	Input @exp_ip/@exp_cidr is conflicts to at lease one known network and
 * 	 	new network is suggested in same parameters.
 *     		New cidr may smaller, larger network, than original value.  It's safe to use original cidr.
 * 	 0:	@exp_ip/@exp_cidr is not conflicts with any known networks.
 * 	-1:	Invalid parameter.
 * 	-2:	'/' absent.
 * 	-3:	illegal IPv4 address or illegal mask\
 * 	-4:	Can't parse IPv4 address
 * 	-5:	Can't parse netmask
 *      <0:	Another error, come from test_one_class().
 */
int test_and_get_free_char_network(int t_class, char *ip_cidr_str, uint32_t excl)
{
	int r, v[4], c;
	in_addr_t mask, mask_type = 0;
	uint32_t exp_ip, exp_cidr;
	char *p, ip_str[sizeof("192.168.100.200XXX")], mask_str[sizeof("255.255.255.255XXX")];

	if (!ip_cidr_str)
		return -1;

	/* Convert char IP/mask to uint32 */
	if (!(p = strchr(ip_cidr_str, '/')))
		return -2;

	strlcpy(ip_str, ip_cidr_str, min(p - ip_cidr_str + 1, sizeof(ip_str)));
	strlcpy(mask_str, p + 1, sizeof(mask_str));
	if (illegal_ipv4_address(ip_str) || illegal_ipv4_netmask(mask_str))
		return -3;

	if ((r = sscanf(ip_str, "%d.%d.%d.%d", &v[0], &v[1], &v[2], &v[3])) != 4)
		return -4;

	exp_ip = (v[0] << 24) | (v[1] << 16) | (v[2] << 8) | v[3];
	if ((strchr(mask_str, '.')) != NULL) {
		/* A.B.C.D */
		if ((mask = inet_network(mask_str)) == -1)
			return -5;
		c = 0;
		while (!(mask & 1)) {
			c++;
			mask >>= 1;
		}
		exp_cidr = 32 - c;
		if (exp_cidr <= 0 || exp_cidr > 32)
			return -5;
		mask_type = 1;
	} else {
		exp_cidr = safe_atoi(mask_str);
	}
	r = test_and_get_free_uint_network(t_class, &exp_ip, exp_cidr, excl);

	/* Convert uint32 to char if new network is suggested. */
	if (r == 1) {
		if (!mask_type) {
			/* Report netmask in CIDR format. */
			snprintf(ip_cidr_str, 32, "%d.%d.%d.%d/%d",
				(exp_ip >> 24) & 0xFF, (exp_ip >> 16) & 0xFF,
				(exp_ip >> 8) & 0xFF, exp_ip & 0xFF, exp_cidr);
		} else {
			/* Report netmask in A.B.C.D format. */
			mask = get_netmask(exp_cidr);
			snprintf(ip_cidr_str, 32, "%d.%d.%d.%d/%d.%d.%d.%d",
				(exp_ip >> 24) & 0xFF, (exp_ip >> 16) & 0xFF,
				(exp_ip >> 8) & 0xFF, exp_ip & 0xFF,
				(mask >> 24) & 0xFF, (mask >> 16) & 0xFF,
				(mask >> 8) & 0xFF, mask & 0xFF);
		}
	}

	return r;
}

#endif /* RTCONFIG_COOVACHILLI || RTCONFIG_PORT_BASED_VLAN || RTCONFIG_TAGGED_BASED_VLAN */

/**
 * Return first/lowest configured and connected WAN unit.
 * @return:	WAN_UNIT_FIRST ~ WAN_UNIT_MAX
 */
enum wan_unit_e get_first_connected_public_wan_unit(void)
{
	int i, wan_unit = WAN_UNIT_MAX;
	int wan_public = 0;
	char wan_ip[sizeof("wanx_ipaddr")];
	char prefix[sizeof("wanXXXXXX_")], link[sizeof("link_wanXXXXXX")];

	for (i = WAN_UNIT_FIRST; i < WAN_UNIT_MAX; ++i) {
		if (get_dualwan_by_unit(i) == WANS_DUALWAN_IF_NONE || !is_wan_connect(i))
			continue;

		/* If the WAN unit is configured as static IP, check link status too. */
		snprintf(prefix, sizeof(prefix), "wan%d_", i);
		if (nvram_pf_match(prefix, "proto", "static")) {
			if (!i)
				strlcpy(link, "link_wan", sizeof(link));
			else
				snprintf(link, sizeof(link), "link_wan%d", i);

			if (!nvram_get_int(link))
				continue;
		}

		snprintf(wan_ip, sizeof(wan_ip), "wan%d_ipaddr", i);
		wan_public = is_private_subnet(nvram_safe_get(wan_ip));
		if(wan_public) // wan_public = 0 is public IP, wan_public = 1, 2, 3, 4 is private IP.
			continue;
		wan_unit = i;
		break;
	}
	if(WAN_UNIT_MAX == i)
		return WAN_UNIT_NONE;
	else
		return wan_unit;
}

int get_wan_proto(char *prefix)
{
	static const struct {
		char *name;
		int service;
	} services[] = {
		{ "dhcp",	WAN_DHCP },
		{ "static",	WAN_STATIC },
		{ "pppoe",	WAN_PPPOE },
		{ "pptp",	WAN_PPTP },
		{ "l2tp",	WAN_L2TP },
		{ "bridge",	WAN_BRIDGE },
#ifdef RTCONFIG_SOFTWIRE46
		{ "lw4o6",	WAN_LW4O6 },
		{ "map-e",	WAN_MAPE },
		{ "v6plus",	WAN_V6PLUS },
#endif
		{ NULL }
	};
	char tmp[100];
	char proto[8] = { 0 };
	int i;

	strlcpy(proto, nvram_safe_get(strlcat_r(prefix, "proto", tmp, sizeof(tmp))), sizeof(proto));
	for (i = 0; services[i].name; i++) {
		if (strcmp(proto, services[i].name) == 0)
			return services[i].service;
	}
	return WAN_DISABLED;
}

int get_ipv4_service_by_unit(int unit)
{
	char prefix[sizeof("wanXXXXXXXXXX_")];

	snprintf(prefix, sizeof(prefix), "wan%d_", unit);
	return get_wan_proto(prefix);
}

int get_ipv4_service(void)
{
	return get_ipv4_service_by_unit(wan_primary_ifunit());
}

#ifdef RTCONFIG_IPV6
char *ipv6_nvname_by_unit(const char *name, int unit)
{
	static char name_ipv6[128];

	if (strncmp(name, "ipv6_", sizeof("ipv6_") - 1) != 0) {
		_dprintf("Wrong ipv6 nvram namespace: \"%s\"\n", name);
		return (char *)name;
	}

	if (unit == 0)
		return (char *)name;

	snprintf(name_ipv6, sizeof(name_ipv6), "ipv6%d_%s", unit, name + sizeof("ipv6_") - 1);

	return name_ipv6;
}

char *ipv6_nvname(const char *name)
{
	return ipv6_nvname_by_unit(name, wan_primary_ifunit_ipv6());
}

int get_ipv6_service_by_unit(int unit)
{
	static const struct {
		char *name;
		int service;
	} services[] = {
		{ "dhcp6",	IPV6_NATIVE_DHCP },
#ifdef RTCONFIG_6RELAYD
		{ "ipv6pt",	IPV6_PASSTHROUGH },
		{ "flets",	IPV6_PASSTHROUGH },
#endif
		{ "6to4",	IPV6_6TO4 },
		{ "6in4",	IPV6_6IN4 },
		{ "6rd",	IPV6_6RD },
		{ "other",	IPV6_MANUAL },
		{ "static6",	IPV6_MANUAL }, /* legacy */
		{ NULL }
	};
	char *value;
	int i;

	value = nvram_safe_get(ipv6_nvname_by_unit("ipv6_service", unit));
	for (i = 0; services[i].name; i++) {
		if (strcmp(value, services[i].name) == 0)
			return services[i].service;
	}
	return IPV6_DISABLED;
}

int get_ipv6_service(void)
{
	return get_ipv6_service_by_unit(wan_primary_ifunit_ipv6());
}

const char *ipv6_router_address(struct in6_addr *in6addr)
{
	char *p;
	struct in6_addr addr;
	static char addr6[INET6_ADDRSTRLEN];

	addr6[0] = '\0';
	memset(&addr, 0, sizeof(addr));

	if ((p = nvram_get(ipv6_nvname("ipv6_rtr_addr"))) && *p) {
		inet_pton(AF_INET6, p, &addr);
	} else if ((p = nvram_get(ipv6_nvname("ipv6_prefix"))) && *p) {
		inet_pton(AF_INET6, p, &addr);
		addr.s6_addr16[7] = htons(0x0001);
	} else
		return addr6;

	inet_ntop(AF_INET6, &addr, addr6, sizeof(addr6));
	if (in6addr)
		memcpy(in6addr, &addr, sizeof(addr));

	return addr6;
}

// trim useless 0 from IPv6 address
const char *ipv6_address(const char *ipaddr6)
{
	struct in6_addr addr;
	static char addr6[INET6_ADDRSTRLEN];

	addr6[0] = '\0';

	if (inet_pton(AF_INET6, ipaddr6, &addr) > 0)
		inet_ntop(AF_INET6, &addr, addr6, sizeof(addr6));

	return addr6;
}

// extract prefix from configured IPv6 address
const char *ipv6_prefix(struct in6_addr *in6addr)
{
	static char prefix[INET6_ADDRSTRLEN];
	struct in6_addr addr;
	int i, r;

	prefix[0] = '\0';
	memset(&addr, 0, sizeof(addr));

	if (inet_pton(AF_INET6, nvram_safe_get(ipv6_nvname("ipv6_rtr_addr")), &addr) > 0) {
		r = nvram_get_int(ipv6_nvname("ipv6_prefix_length")) ? : 64;
		for (r = 128 - r, i = 15; r > 0; r -= 8) {
			if (r >= 8)
				addr.s6_addr[i--] = 0;
			else
				addr.s6_addr[i--] &= (0xff << r);
		}
		inet_ntop(AF_INET6, &addr, prefix, sizeof(prefix));
	}

	if (in6addr)
		memcpy(in6addr, &addr, sizeof(addr));

	return prefix;
}

#if 0 /* unused */
const char *ipv6_prefix(const char *ifname)
{
	return getifaddr(ifname, AF_INET6, GIF_PREFIX) ? : "";
}
 
int ipv6_prefix_len(const char *ifname)
{
	const char *value;

	value = getifaddr(ifname, AF_INET6, /*GIF_PREFIX |*/ GIF_PREFIXLEN);
	if (value == NULL)
		return 0;

	value = strchr(value, '/');
	if (value)
		return atoi(value + 1);

	return 128;
}
#endif

void reset_ipv6_linklocal_addr(const char *ifname, int flush)
{
	static char buf[INET6_ADDRSTRLEN];
	struct in6_addr addr;
	struct ifreq ifr;
	char *mac;
	int fd;

	if (!ifname ||
	    !strcmp(ifname, "lo") || !strncmp(ifname, "ppp", 3))
		return;

	if ((fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
		return;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
	mac = (ioctl(fd, SIOCGIFHWADDR, &ifr) < 0) ? NULL : ifr.ifr_hwaddr.sa_data;
	close(fd);

	if (mac == NULL)
		return;

	addr.s6_addr32[0] = htonl(0xfe800000);
	addr.s6_addr32[1] = 0;
	memcpy(&addr.s6_addr[8], mac, 3);
	memcpy(&addr.s6_addr[13], mac + 3, 3);
	addr.s6_addr[8] ^= 0x02;
	addr.s6_addr[11] = 0xff;
	addr.s6_addr[12] = 0xfe;

	if (flush)
		eval("ip", "-6", "addr", "flush", "dev", (char *) ifname);
	if (inet_ntop(AF_INET6, &addr, buf, sizeof(buf))) {
		strlcat(buf, "/64", sizeof(buf));
		eval("ip", "-6", "addr", "add", buf, "dev", (char *) ifname, "scope", "link");
	}
}

int with_ipv6_linklocal_addr(const char *ifname)
{
	return (getifaddr(ifname, AF_INET6, GIF_LINKLOCAL) != NULL);
}

static int ipv6_expand(char *buf, char *str, struct in6_addr *addr)
{
	char *dst = buf;
	char *src = str;
	int i;

	for (i = 1; dst < src && *src; i++) {
		*dst++ = *src++;
		if (i % 4 == 0) *dst++ = ':';
	}
	return inet_pton(AF_INET6, buf, addr);
}

const char *ipv6_gateway_address(void)
{
	static char buf[INET6_ADDRSTRLEN] = "";
	FILE *fp;
	struct in6_addr addr;
	char dest[41], nexthop[41], dev[17];
	int mask, prefix, maxprefix, flags;
	unsigned int metric, minmetric = minmetric;

	fp = fopen("/proc/net/ipv6_route", "r");
	if (fp == NULL) {
		perror("/proc/net/ipv6_route");
		return NULL;
	}

	maxprefix = -1;
	while (fscanf(fp, "%32s%x%*s%*x%32s%x%*x%*x%x%16s\n",
		      &dest[7], &prefix, &nexthop[7], &metric, &flags, dev) == 6) {
		/* Skip interfaces that are down and host routes */
		if ((flags & (RTF_UP | RTF_HOST | RTF_REJECT)) != RTF_UP)
			continue;

		/* Skip dst not in "::/0 - 2000::/3" */
		if (prefix > 3)
			continue;
		if (ipv6_expand(dest, &dest[7], &addr) <= 0)
			continue;
		mask = htons((0xffff0000 >> prefix) & 0xffff);
		if ((addr.s6_addr16[0] & mask) != (htons(0x2000) & mask))
			continue;

		/* Skip dups & worse routes */
		if ((maxprefix > prefix) ||
		    (maxprefix == prefix && minmetric <= metric))
			continue;

		if (flags & RTF_GATEWAY) {
			if (ipv6_expand(nexthop, &nexthop[7], &addr) <= 0)
				continue;
			inet_ntop(AF_INET6, &addr, buf, sizeof(buf));
		} else
			strlcpy(buf, "::", sizeof(buf));
		maxprefix = prefix;
		minmetric = metric;

		if (prefix == 0)
			break;
	}
	fclose(fp);

	return (maxprefix < 0) ? NULL : buf;
}
#endif

int wl_client(int unit, int subunit)
{
	char *mode = nvram_safe_get(wl_nvname("mode", unit, subunit));

	return ((strcmp(mode, "sta") == 0) || (strcmp(mode, "wet") == 0) || (strcmp(mode, "psta") == 0) || (strcmp(mode, "psr") == 0));
}

int foreach_wif(int include_vifs, void *param,
	int (*func)(int idx, int unit, int subunit, void *param))
{
	char ifnames[256];
	char name[64], ifname[64], *next = NULL;
	int unit = -1, subunit = -1;
	int i;
	int ret = 0;

	snprintf(ifnames, sizeof(ifnames), "%s %s",
		 nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
	remove_dups(ifnames, sizeof(ifnames));

	i = 0;
	foreach(name, ifnames, next) {
		SKIP_ABSENT_FAKE_IFACE(name);
		if (nvifname_to_osifname(name, ifname, sizeof(ifname)) != 0)
			continue;

#ifdef CONFIG_BCMWL5
		if (wl_probe(ifname) || wl_ioctl(ifname, WLC_GET_INSTANCE, &unit, sizeof(unit)))
			continue;
#endif

		// Convert eth name to wl name
		if (osifname_to_nvifname(name, ifname, sizeof(ifname)) != 0)
			continue;

#ifdef CONFIG_BCMWL5
		// Slave intefaces have a '.' in the name
		if (strchr(ifname, '.') && !include_vifs)
			continue;
#endif

		if (get_ifname_unit(ifname, &unit, &subunit) < 0)
			continue;

		ret |= func(i++, unit, subunit, param);
	}
	return ret;
}

void notice_set(const char *path, const char *format, ...)
{
	char p[256];
	char buf[2048];
	va_list args;

	va_start(args, format);
	vsnprintf(buf, sizeof(buf), format, args);
	va_end(args);

	mkdir("/var/notice", 0755);
	snprintf(p, sizeof(p), "/var/notice/%s", path);
	f_write_string(p, buf, 0, 0);
	if (buf[0]) syslog(LOG_INFO, "notice[%s]: %s", path, buf);
}


//	#define _x_dprintf(args...)	syslog(LOG_DEBUG, args);
#define _x_dprintf(args...)	do { } while (0);

// -----------------------------------------------------------------------------
#define ACT_COMM_LEN	20
struct action_s {
	char comm[ACT_COMM_LEN];
	pid_t pid;
	int action;
};

void set_action(int a)
{
	int r = 3;
	struct action_s act;
	char *s, *p, stat[sizeof("/proc/XXXXXX/statXXXXXX")];

	act.action = a;
	act.pid = getpid();
	memset(act.comm, 0, sizeof(act.comm));
	snprintf(stat, sizeof(stat), "/proc/%d/stat", act.pid);
	s = file2str(stat);
	if (s) {
		if ((p = strrchr(s, ')')) != NULL)
			*p = '\0';
		if ((p = strchr(s, '(')) != NULL)
			snprintf(act.comm, sizeof(act.comm), "%s", ++p);
		free(s);
		s = (p && *act.comm) ? act.comm : NULL;
	}
	if (!s)
		snprintf(act.comm, sizeof(act.comm), "%d <UNKNOWN>", act.pid);
	_dprintf("%d: set_action %d\n", getpid(), act.action);
	while (f_write_excl(ACTION_LOCK, &act, sizeof(act), 0, 0600) != sizeof(act)) {
		sleep(1);
		if (--r == 0) return;
	}
	if (a != ACT_IDLE) sleep(2);
}

static int __check_action(struct action_s *pa)
{
	int r = 3;
	struct action_s act;

	while (f_read_excl(ACTION_LOCK, &act, sizeof(act)) != sizeof(act)) {
		sleep(1);
		if (--r == 0) return ACT_UNKNOWN;
	}
	if (pa)
		memcpy(pa, &act, sizeof(act));
	_dprintf("%d: check_action %d: %d(%s)\n", getpid(), act.action, act.pid, act.comm);

	return act.action;
}

int check_action(void)
{
	return __check_action(NULL);
}

/* wait_action_idle(int n)
 *
 * n: the number (count down value) to try
 *
 * return value
 * 	n: success and return the remainng value of n.
 * 	0: fail
 */
int wait_action_idle(int n)
{
	int r;
	struct action_s act;

	memset(&act, 0, sizeof(act));
	while (n > 0) {
		act.pid = 0;
		if (__check_action(&act) == ACT_IDLE) return n;
		if (act.pid > 0 && !process_exists(act.pid)) {
			if (!(r = unlink(ACTION_LOCK)) || errno == ENOENT) {
				_dprintf("Terminated process, pid %d %s, hold action lock %d !!!\n",
					act.pid, act.comm, act.action);
				return n;
			}
			_dprintf("Remove " ACTION_LOCK " failed. errno %d (%s)\n", errno, strerror(errno));
		}
		sleep(1);
		n--;
	}
	_dprintf("pid %d %s hold action lock %d !!!\n", act.pid, act.comm, act.action);
	return 0;
}

const char *get_wanip(void)
{
	char tmp[100], prefix[sizeof("wanXXXXXXXXXX_")];

	snprintf(prefix, sizeof(prefix), "wan%d_", wan_primary_ifunit());

	return nvram_safe_get(strlcat_r(prefix, "ipaddr", tmp, sizeof(tmp)));
}

int get_wanstate(void)
{
	char tmp[100], prefix[sizeof("wanXXXXXXXXXX_")];

	snprintf(prefix, sizeof(prefix), "wan%d_", wan_primary_ifunit());

	return nvram_get_int(strlcat_r(prefix, "state_t", tmp, sizeof(tmp)));
}

const char *get_wanface(void)
{
	return get_wan_ifname(wan_primary_ifunit());
}

#ifdef RTCONFIG_IPV6
const char *get_wan6face(void)
{
	return get_wan6_ifname(wan_primary_ifunit_ipv6());
}

int update_6rd_info(void)
{
	char tmp[100], prefix[]="wanXXXXX_";
	char addr6[INET6_ADDRSTRLEN + 1], *value;
	struct in6_addr addr;

	if (get_ipv6_service() != IPV6_6RD || !nvram_get_int(ipv6_nvname("ipv6_6rd_dhcp")))
		return -1;

	snprintf(prefix, sizeof(prefix), "wan%d_", wan_primary_ifunit_ipv6());

	value = nvram_safe_get(strlcat_r(prefix, "6rd_prefix", tmp, sizeof(tmp)));
	if (*value ) {
		/* try to compact IPv6 prefix */
		if (inet_pton(AF_INET6, value, &addr) > 0)
			value = (char *) inet_ntop(AF_INET6, &addr, addr6, sizeof(addr6));
		nvram_set(ipv6_nvname("ipv6_6rd_prefix"), value);
		nvram_set(ipv6_nvname("ipv6_6rd_router"), nvram_safe_get(strlcat_r(prefix, "6rd_router", tmp, sizeof(tmp))));
		nvram_set(ipv6_nvname("ipv6_6rd_prefixlen"), nvram_safe_get(strlcat_r(prefix, "6rd_prefixlen", tmp, sizeof(tmp))));
		nvram_set(ipv6_nvname("ipv6_6rd_ip4size"), nvram_safe_get(strlcat_r(prefix, "6rd_ip4size", tmp, sizeof(tmp))));
		return 1;
	}

	return 0;
}
#endif

const char *_getifaddr(const char *ifname, int family, int flags, char *buf, int size)
{
	struct ifaddrs *ifap, *ifa;
	union {
#ifdef RTCONFIG_IPV6
		struct in6_addr in6;
#endif
		struct in_addr in;
	} addrbuf;
	unsigned char *addr, *netmask, *paddr, *pnetmask;
	unsigned char len, maxlen;

	if (getifaddrs(&ifap) != 0) {
		_dprintf("getifaddrs failed: %s\n", strerror(errno));
		return NULL;
	}

	for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr == NULL ||
		    ifa->ifa_addr->sa_family != family ||
		    strncmp(ifa->ifa_name, ifname, IFNAMSIZ) != 0)
			continue;

		switch (ifa->ifa_addr->sa_family) {
#ifdef RTCONFIG_IPV6
		case AF_INET6:
			addr = (void *) &((struct sockaddr_in6 *) ifa->ifa_addr)->sin6_addr;
			if (IN6_IS_ADDR_LINKLOCAL(addr) ^ !!(flags & GIF_LINKLOCAL))
				continue;
			netmask = (void *) &((struct sockaddr_in6 *) ifa->ifa_netmask)->sin6_addr;
			maxlen = sizeof(struct in6_addr) * 8;
			break;
#endif
		case AF_INET:
			addr = (void *) &((struct sockaddr_in *) ifa->ifa_addr)->sin_addr;
			netmask = (void *) &((struct sockaddr_in *) ifa->ifa_netmask)->sin_addr;
			maxlen = sizeof(struct in_addr) * 8;
			break;
		default:
			continue;
		}

		if ((flags & GIF_PREFIX) && addr && netmask) {
			paddr = addr = memcpy(&addrbuf, addr, maxlen / 8);
			pnetmask = netmask;
			for (len = 0; len < maxlen; len += 8)
				*paddr++ &= *pnetmask++;
		}

		if (addr && inet_ntop(ifa->ifa_addr->sa_family, addr, buf, size) != NULL) {
			if ((flags & GIF_PREFIXLEN) && netmask) {
				for (len = 0; len < maxlen && *netmask == 0xff; len += 8)
					netmask++;
				if (len < maxlen && *netmask) {
					switch (*netmask) {
					case 0xfe: len += 7; break;
					case 0xfc: len += 6; break;
					case 0xf8: len += 5; break;
					case 0xf0: len += 4; break;
					case 0xe0: len += 3; break;
					case 0xc0: len += 2; break;
					case 0x80: len += 1; break;
					default:
						_dprintf("getifaddrs netmask error: %x\n", *netmask);
						goto error;
					}
				}
				if (len < maxlen) {
					netmask = memchr(buf, 0, size);
					if (netmask)
						snprintf((char *) netmask, size - ((char *) netmask - buf), "/%d", len);
				}
			}
			freeifaddrs(ifap);
			return buf;
		}
	}

error:
	freeifaddrs(ifap);
	return NULL;
}

const char *getifaddr(const char *ifname, int family, int flags)
{
	static char buf[INET6_ADDRSTRLEN];

	return _getifaddr(ifname, family, flags, buf, sizeof(buf));
}

int is_intf_up(const char* ifname)
{
	struct ifreq ifr;
	int sfd;
	int ret = 0;

	if (!ifname || !strlen(ifname))
		return 0;

	if (!((sfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0))
	{
		strlcpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
		if (ioctl(sfd, SIOCGIFFLAGS, &ifr))
			ret = -1;			/* interface not exist */
		else if((ifr.ifr_flags & IFF_UP))
			ret = 1;			/* interface exist and up */

		close(sfd);
	}

	return ret;
}

int aimesh_re_mode(void)
{
#ifdef RTCONFIG_LANTIQ
	return (sw_mode() == SW_MODE_AP && nvram_get_int("wlc_psta") == 2);
#else
	return 0;
#endif
}

char *wl_nvprefix(char *prefix, int prefix_size, int unit, int subunit)
{
	if(unit < 0)
		strlcpy(prefix, "wl_", prefix_size);
#ifdef RTCONFIG_LANTIQ
	else if(client_mode() && unit == nvram_get_int("wlc_band"))
		snprintf(prefix, prefix_size, "wl%d.%d_", unit, 1);
	else if(aimesh_re_mode())
		snprintf(prefix, prefix_size, "wl%d.%d_", unit, 1);
#endif
	else if(subunit > 0)
		snprintf(prefix, prefix_size, "wl%d.%d_", unit, subunit);
	else
		snprintf(prefix, prefix_size, "wl%d_", unit);

	return prefix;
}

char *wl_nvname(const char *nv, int unit, int subunit)
{
	static char tmp[128];
	char prefix[] = "wlXXXXXXXXXXXXX_";

	wl_nvprefix(prefix, sizeof(prefix), unit, subunit);

	return strlcat_r(prefix, nv, tmp, sizeof(tmp));
}

char *wl_nband_name(const char *nband)
{
	if (!nband || *nband == '\0') {
		dbg("%s: invalid nband %p\n", __func__, nband);
		return "2.4 GHz";
	}

	if (!strcmp(nband, "2"))
		return "2.4 GHz";
	else if (!strcmp(nband, "1"))
		return "5 GHz";
	else if (!strcmp(nband, "6"))
		return "60 GHz";
	else
		dbg("%s: unknown nband [%s]\n", __func__, nband);

	return "2.4 GHz";
}

// -----------------------------------------------------------------------------

int mtd_getinfo(const char *mtdname, int *part, int *size)
{
	FILE *f;
	char s[256];
	char t[256];
	int r;

	r = 0;
	if ((strlen(mtdname) < 128)) { // && (strcmp(mtdname, "pmon") != 0)) { //Yau dbg
		snprintf(t, sizeof(t), "\"%s\"", mtdname);
		if ((f = fopen("/proc/mtd", "r")) != NULL) {
			while (fgets(s, sizeof(s), f) != NULL) {
				if ((sscanf(s, "mtd%d: %x", part, size) == 2) && (strstr(s, t) != NULL)) {
					// don't accidentally mess with bl (0)
					if (*part >= 0) r = 1; //Yau > -> >=
					r =1;
					break;
				}
			}
			fclose(f);
		}
	}
	if (!r) {
		*size = 0;
		*part = -1;
	}
	return r;
}

#if defined(RTCONFIG_UBIFS)
#define UBI_SYSFS_DIR	"/sys/class/ubi"
/* return device number, volume id, and volume size in bytes
 * @return:
 *      1:	ubi volume not found
 * 	0:	success
 *     -1:	invalid parameter
 *     -2:	UBI not exist (open /sys/class/ubi failed)
 *     <0:	error
 */
int ubi_getinfo(const char *ubiname, int *dev, int *part, int *size)
{
	DIR *dir;
	int d, p, l, cmp, ret = 1;
	char *s1, *s2, path[PATH_MAX];
	struct dirent *ent;

	if (!ubiname || *ubiname == '\0' || !dev || !part || !size)
		return -1;

	if ((dir = opendir(UBI_SYSFS_DIR)) == NULL)
		return -2;

	while (ret && (ent = readdir(dir)) != NULL) {
		if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..") || strncmp(ent->d_name, "ubi", 3))
			continue;
		if (sscanf(ent->d_name, "ubi%d_%d", &d, &p) != 2)
			continue;

		snprintf(path, sizeof(path), "%s/ubi%d_%d/name", UBI_SYSFS_DIR, d, p);
		if (!(s1 = file2str(path)))
			continue;
		/* remove tailed line-feed character */
		l = strlen(s1);
		if (*(s1+l-1) == '\xa')
			*(s1+l-1) = '\0';
		cmp = strcmp(ubiname, s1);
		free(s1);
		if (cmp)
			continue;

		snprintf(path, sizeof(path), "%s/ubi%d_%d/data_bytes", UBI_SYSFS_DIR, d, p);
		if (!(s2 = file2str(path)))
			continue;
		*dev = d;
		*part = p;
		*size = atoi(s2);
		free(s2);
		ret = 0;
	}
	closedir(dir);

	return ret;
}
#endif

// -----------------------------------------------------------------------------

/* 
 * Get the value of an NVRAM variable, using supplied buffer.
 * @param	name	name of variable to get
 * @param	buf	local buffer for the value
 * @param	bufsize	local buffer size
 * @return	value of variable copied to local buffer or
 *		NULL if undefined.
 */
char *nvram_get_r(const char *name, char *buf, size_t buflen)
{
	char *v = nvram_get(name);

	if (v && buf) {
		strlcpy(buf, v, buflen);
		return buf;
	}

	return v;
}

/**
 * Combine prefix and name before nvram_get().
 * @prefix:
 * @name:
 * @return:
 */
char *nvram_pf_get(const char *prefix, const char *name)
{
	char tmp[128], *t = tmp, *v;
	size_t size;

	if (!prefix || !name || *name == '\0')
		return NULL;

	size = strlen(prefix) + strlen(name) + 1;
	if (size > sizeof(tmp)) {
		t = malloc(size);
		if (!t)
			return NULL;
	}

	v = nvram_get(strlcat_r(prefix, name, tmp, sizeof(tmp)));

	if (t != tmp)
		free(t);

	return v;
}

/**
 * Combine prefix and name before nvram_set().
 * @prefix:
 * @name:
 * @value:
 * @return:
 */
int nvram_pf_set(const char *prefix, const char *name, const char *value)
{
	char tmp[128], *t = tmp;
	size_t size;
	int r;

	if (!prefix || !name || *name == '\0')
		return -EINVAL;

	size = strlen(prefix) + strlen(name) + 1;
	if (size > sizeof(tmp)) {
		t = malloc(size);
		if (!t)
			return -ENOMEM;
	}

	r = nvram_set(strlcat_r(prefix, name, tmp, sizeof(tmp)), value);

	if (t != tmp)
		free(t);

	return r;
}

int nvram_get_int(const char *key)
{
	return atoi(nvram_safe_get(key));
}

int nvram_pf_get_int(const char *prefix, const char *key)
{
	return atoi(nvram_pf_safe_get(prefix, key));
}

int nvram_set_int(const char *key, int value)
{
	char nvramstr[16];

	snprintf(nvramstr, sizeof(nvramstr), "%d", value);
	return nvram_set(key, nvramstr);
}

int nvram_pf_set_int(const char *prefix, const char *key, int value)
{
	char nvramstr[16];

	snprintf(nvramstr, sizeof(nvramstr), "%d", value);
	return nvram_pf_set(prefix, key, nvramstr);
}

/**
 * Match an prefix NVRAM variable.
 */
int nvram_pf_match(char *prefix, char *name, char *match)
{
	const char *value = nvram_pf_get(prefix, name);
	return (value && !strcmp(value, match));
}

/**
 * Inversely match an prefix NVRAM variable.
 */
int nvram_pf_invmatch(char *prefix, char *name, char *invmatch)
{
	const char *value = nvram_pf_get(prefix, name);
	return (value && strcmp(value, invmatch));
}

double nvram_get_double(const char *key)
{
	return atof(nvram_safe_get(key));
}

int nvram_set_double(const char *key, double value)
{
	char nvramstr[33];

	snprintf(nvramstr, sizeof(nvramstr), "%.9g", value);
	return nvram_set(key, nvramstr);
}

#ifdef HND_ROUTER
/**
 * Functions that split settings accross multiple variables
 */
char *nvram_split_get(const char *key, char *buffer, int maxlen, int maxinst)
{
	char nvname[64];
	int i;

	strlcpy(buffer, nvram_safe_get(key), maxlen);

	for (i=1; i <= maxinst; ++i) {
		snprintf(nvname, sizeof (nvname), "%s%d", key, i);
		strlcat(buffer, nvram_safe_get(nvname), maxlen);
	}

	return buffer;
}

int nvram_split_set(const char *key, char *value, int size, int maxinst)
{
	char nvname[64];
	char *piece, *ptr;
	int valuelen, i;

	piece = malloc(size + 1);
	if (piece == NULL)
		return -1;
	piece[size] = '\0';

	valuelen = strlen(value);
	ptr = value;

	strncpy(piece, ptr, size);
	nvram_set(key, piece);
	ptr += size;

	if (valuelen <= size) {
		free(piece);
		return 1;
	}

	for (i=1; i <= maxinst; ++i) {
		strncpy(piece, ptr, size);

		snprintf(nvname, sizeof (nvname), "%s%d", key, i);
		nvram_set(nvname, piece);

		ptr += size;
		if (ptr >= value + valuelen) {
			free(piece);
			return i;
		}
	}

	free(piece);

	return i;
}
#endif

int nvram_get_hex(const char *key)
{
	return strtol(nvram_safe_get(key), NULL, 16);
}

int nvram_set_hex(const char *key, int value)
{
	char nvramstr[16];

	snprintf(nvramstr, sizeof(nvramstr), "%x", value);
	return nvram_set(key, nvramstr);
}

int nvram_valid_get_int(const char *key, int min, int max, int def)
{
	int ret = nvram_get_int(key);
	return (min <= ret && ret <= max) ? ret : def;
}

#if defined(RTCONFIG_HTTPS)
int nvram_get_file(const char *key, const char *fname, int max)
{
	int n;
	char *p;
	char *b;
	int r;

	r = 0;
	p = nvram_safe_get(key);
	n = strlen(p);
	if (n <= max) {
		if ((b = malloc(base64_decoded_len(n) + 128)) != NULL) {
			n = base64_decode(p, (unsigned char *) b, n);
			if (n > 0) r = (f_write(fname, b, n, 0, 0644) == n);
			free(b);
		}
	}
	return r;
/*
	char b[3500];
	int n;
	char *p;

	p = nvram_safe_get(key);
	n = strlen(p);
	if (n <= max) {
		n = base64_decode(p, b, n);
		if (n > 0) return (f_write(fname, b, n, 0, 0700) == n);
	}
	return 0;
*/
}

int nvram_set_file(const char *key, const char *fname, int max)
{
	char *in;
	char *out;
	long len;
	int n;
	int r;

	if ((len = f_size(fname)) > max) return 0;
	max = (int)len;
	r = 0;
	if (f_read_alloc(fname, &in, max) == max) {
		if ((out = malloc(base64_encoded_len(max) + 128)) != NULL) {
			n = base64_encode((unsigned char *) in, out, max);
			out[n] = 0;
			nvram_set(key, out);
			free(out);
			r = 1;
		}
		free(in);
	}
	return r;
/*
	char a[3500];
	char b[7000];
	int n;

	if (((n = f_read(fname, &a, sizeof(a))) > 0) && (n <= max)) {
		n = base64_encode(a, b, n);
		b[n] = 0;
		nvram_set(key, b);
		return 1;
	}
	return 0;
*/
}
#endif

int nvram_contains_word(const char *key, const char *word)
{
	return (find_word(nvram_safe_get(key), word) != NULL);
}

int nvram_is_empty(const char *key)
{
	char *p;
	return (((p = nvram_get(key)) == NULL) || (*p == 0));
}

void nvram_commit_x(void)
{
	if (!nvram_get_int("debug_nocommit")) nvram_commit();
}

void chld_reap(int sig)
{
	while (waitpid(-1, NULL, WNOHANG) > 0) {}
}

static uint32 crc_table[256]={ 0x00000000,0x77073096,0xEE0E612C,0x990951BA,
0x076DC419,0x706AF48F,0xE963A535,0x9E6495A3,0x0EDB8832,0x79DCB8A4,0xE0D5E91E,
0x97D2D988,0x09B64C2B,0x7EB17CBD,0xE7B82D07,0x90BF1D91,0x1DB71064,0x6AB020F2,
0xF3B97148,0x84BE41DE,0x1ADAD47D,0x6DDDE4EB,0xF4D4B551,0x83D385C7,0x136C9856,
0x646BA8C0,0xFD62F97A,0x8A65C9EC,0x14015C4F,0x63066CD9,0xFA0F3D63,0x8D080DF5,
0x3B6E20C8,0x4C69105E,0xD56041E4,0xA2677172,0x3C03E4D1,0x4B04D447,0xD20D85FD,
0xA50AB56B,0x35B5A8FA,0x42B2986C,0xDBBBC9D6,0xACBCF940,0x32D86CE3,0x45DF5C75,
0xDCD60DCF,0xABD13D59,0x26D930AC,0x51DE003A,0xC8D75180,0xBFD06116,0x21B4F4B5,
0x56B3C423,0xCFBA9599,0xB8BDA50F,0x2802B89E,0x5F058808,0xC60CD9B2,0xB10BE924,
0x2F6F7C87,0x58684C11,0xC1611DAB,0xB6662D3D,0x76DC4190,0x01DB7106,0x98D220BC,
0xEFD5102A,0x71B18589,0x06B6B51F,0x9FBFE4A5,0xE8B8D433,0x7807C9A2,0x0F00F934,
0x9609A88E,0xE10E9818,0x7F6A0DBB,0x086D3D2D,0x91646C97,0xE6635C01,0x6B6B51F4,
0x1C6C6162,0x856530D8,0xF262004E,0x6C0695ED,0x1B01A57B,0x8208F4C1,0xF50FC457,
0x65B0D9C6,0x12B7E950,0x8BBEB8EA,0xFCB9887C,0x62DD1DDF,0x15DA2D49,0x8CD37CF3,
0xFBD44C65,0x4DB26158,0x3AB551CE,0xA3BC0074,0xD4BB30E2,0x4ADFA541,0x3DD895D7,
0xA4D1C46D,0xD3D6F4FB,0x4369E96A,0x346ED9FC,0xAD678846,0xDA60B8D0,0x44042D73,
0x33031DE5,0xAA0A4C5F,0xDD0D7CC9,0x5005713C,0x270241AA,0xBE0B1010,0xC90C2086,
0x5768B525,0x206F85B3,0xB966D409,0xCE61E49F,0x5EDEF90E,0x29D9C998,0xB0D09822,
0xC7D7A8B4,0x59B33D17,0x2EB40D81,0xB7BD5C3B,0xC0BA6CAD,0xEDB88320,0x9ABFB3B6,
0x03B6E20C,0x74B1D29A,0xEAD54739,0x9DD277AF,0x04DB2615,0x73DC1683,0xE3630B12,
0x94643B84,0x0D6D6A3E,0x7A6A5AA8,0xE40ECF0B,0x9309FF9D,0x0A00AE27,0x7D079EB1,
0xF00F9344,0x8708A3D2,0x1E01F268,0x6906C2FE,0xF762575D,0x806567CB,0x196C3671,
0x6E6B06E7,0xFED41B76,0x89D32BE0,0x10DA7A5A,0x67DD4ACC,0xF9B9DF6F,0x8EBEEFF9,
0x17B7BE43,0x60B08ED5,0xD6D6A3E8,0xA1D1937E,0x38D8C2C4,0x4FDFF252,0xD1BB67F1,
0xA6BC5767,0x3FB506DD,0x48B2364B,0xD80D2BDA,0xAF0A1B4C,0x36034AF6,0x41047A60,
0xDF60EFC3,0xA867DF55,0x316E8EEF,0x4669BE79,0xCB61B38C,0xBC66831A,0x256FD2A0,
0x5268E236,0xCC0C7795,0xBB0B4703,0x220216B9,0x5505262F,0xC5BA3BBE,0xB2BD0B28,
0x2BB45A92,0x5CB36A04,0xC2D7FFA7,0xB5D0CF31,0x2CD99E8B,0x5BDEAE1D,0x9B64C2B0,
0xEC63F226,0x756AA39C,0x026D930A,0x9C0906A9,0xEB0E363F,0x72076785,0x05005713,
0x95BF4A82,0xE2B87A14,0x7BB12BAE,0x0CB61B38,0x92D28E9B,0xE5D5BE0D,0x7CDCEFB7,
0x0BDBDF21,0x86D3D2D4,0xF1D4E242,0x68DDB3F8,0x1FDA836E,0x81BE16CD,0xF6B9265B,
0x6FB077E1,0x18B74777,0x88085AE6,0xFF0F6A70,0x66063BCA,0x11010B5C,0x8F659EFF,
0xF862AE69,0x616BFFD3,0x166CCF45,0xA00AE278,0xD70DD2EE,0x4E048354,0x3903B3C2,
0xA7672661,0xD06016F7,0x4969474D,0x3E6E77DB,0xAED16A4A,0xD9D65ADC,0x40DF0B66,
0x37D83BF0,0xA9BCAE53,0xDEBB9EC5,0x47B2CF7F,0x30B5FFE9,0xBDBDF21C,0xCABAC28A,
0x53B39330,0x24B4A3A6,0xBAD03605,0xCDD70693,0x54DE5729,0x23D967BF,0xB3667A2E,
0xC4614AB8,0x5D681B02,0x2A6F2B94,0xB40BBE37,0xC30C8EA1,0x5A05DF1B,0x2D02EF8D};

uint32_t crc_calc(uint32_t crc, const char *buf, int len)
{
#if !defined(RTCONFIG_LANTIQ) && !defined(RTCONFIG_ALPINE)
	crc = crc ^ 0xffffffff;
#endif
	while (len-- > 0) {
		crc = crc_table[(crc ^ *((unsigned char *)buf)) & 0xFF] ^ (crc >> 8);
		buf++;
	}
#if !defined(RTCONFIG_LANTIQ) && !defined(RTCONFIG_ALPINE)
	crc = crc ^ 0xffffffff;
#endif
	return crc;
}

// ugly solution and doesn't support wantag
void bcmvlan_models(int model, char *vlan)
{
	switch (model) {
	case MODEL_DSLAC68U:
	case MODEL_RTAC68U:
	case MODEL_RTAC87U:
	case MODEL_RTAC56S:
	case MODEL_RTAC56U:
	case MODEL_RTAC66U:
	case MODEL_RTN66U:
	case MODEL_RTN16:
	case MODEL_RTN18U:
	case MODEL_RTN15U:
	case MODEL_RTAC53U:
	case MODEL_RTAC3200:
	case MODEL_RTAC88U:
	case MODEL_RTAC86U:
	case MODEL_RTAC3100:
	case MODEL_RTAC5300:
	case MODEL_GTAC5300:
	case MODEL_RTAC1200G:
	case MODEL_RTAC1200GP:
		strcpy(vlan, "vlan1");
		break;
	case MODEL_RTN53:
	case MODEL_RTN12:
	case MODEL_RTN12B1:
	case MODEL_RTN12C1:
	case MODEL_RTN12D1:
	case MODEL_RTN12VP:
	case MODEL_RTN12HP:
	case MODEL_RTN12HP_B1:
	case MODEL_RTN14UHP:
	case MODEL_RTN10U:
	case MODEL_RTN10P:
	case MODEL_RTN10D1:
	case MODEL_RTN10PV2:
		strcpy(vlan, "vlan0");
		break;
	default:
		strcpy(vlan, "");
		break;
	}
}

char *get_productid(void)
{
#ifdef RTCONFIG_ODMPID
	char *productid = nvram_safe_get("odmpid");

	if (*productid)
		return productid;
#endif

	return nvram_safe_get("productid");
}

char *get_lan_hostname(void)
{
	char *hostname = nvram_safe_get("lan_hostname");

	if (*hostname && is_valid_hostname(hostname))
		return hostname;

	return get_productid();
}

unsigned long long backup_rx;
unsigned long long backup_tx;
int backup_set = 0;

/* Looking for a ifino_s by interface name.
 * @ifinotbl:
 * @ifname:
 * @return:
 * 	NULL		not found or invalid parameter;
 *  otherwise		pointer to struct ifname_ino
 */
struct ifino_s *ifname_ino_ptr(struct ifname_ino_tbl *ifinotbl, const char *ifname)
{
	int i;
	struct ifino_s *ret = NULL, *p;

	if (!ifinotbl || !ifname || *ifname == '\0')
		return NULL;

	for (i = 0, p = &ifinotbl->items[0]; i < ifinotbl->nr_items; ++i, ++p) {
		if (strcmp(p->ifname, ifname))
			continue;
		ret = p;
		break;
	}

	return ret;
}

/* Get inode of a interface.
 * @ifname:
 * @return:
 * 	0	error
 *  otherwise	inode of @ifname
 */
ino_t get_iface_inode(const char *ifname)
{
	struct stat s;
	char path[sizeof(SYS_CLASS_NET) + 1 + IFNAMSIZ + 6];

	if (!ifname || *ifname == '\0')
		return 0;

	snprintf(path, sizeof(path), "%s/%s", SYS_CLASS_NET, ifname);
	if (stat(path, &s))
		return 0;

	return s.st_ino;
}

static void get_internet_desc(int unit, char *ifname_desc)
{
	if (!ifname_desc)
		return;

#ifdef RTCONFIG_DUALWAN
	if (unit == WAN_UNIT_FIRST) {
		strcpy(ifname_desc, "INTERNET");
	} else {
		sprintf(ifname_desc,"INTERNET%d", unit);
	}
#else
	if (unit == wan_primary_ifunit()) {
		strcpy(ifname_desc, "INTERNET");
	}
#endif	/* RTCONFIG_DUALWAN */
}

static const struct dummy_ifaces_s {
	int exact;	/* 1: true if name equal to ifname; 0: true if name is substring of ifname. */
	char *name;
} dummy_ifaces_list[] = {
	{ 1, "lo" },
	{ 0, "imq" },		/* imq* */
#if defined(RTCONFIG_QCA)
	{ 0, "qca-nss-dev" },	/* qca-nss-dev0~11 */
	{ 0, "wifi" },		/* wifi0, wifi1 */
	{ 0, "sit" },		/* sit0 */
	{ 0, "ip6tnl" },	/* ip6tnl0 */
	{ 1, "miireg" },
	{ 1, "soc0" },
	{ 1, "ipsecdummy" },
#endif

	{ 0, NULL }
};

/** Convert strings like "0 1 15" to uint32_t bitmask 0x8003.
 * @str:	pointer to a string includes one or more number.
 * @return:	bitmask
 */
uint32_t nums_str_to_u32_mask(const char *str)
{
	uint32_t r = 0;
	int b;
	char *next, num[16], tmp[100];

	if (!str || *str == '\0')
		return 0;

	strlcpy(tmp, str, sizeof(tmp));
	foreach (num, tmp, next) {
		b = safe_atoi(num);
		if (b < 0 || b > 31)
			continue;
		r |= 1U << b;
	}

	return r;
}

/*
 * @ifname_desc:	12 bytes character array.
 * @return:
 * 	0:	ignore TX/RX bytes of this interface.
 *  otherwise:	don't ignore TX/RX bytes of this interface.
 */
unsigned int netdev_calc(char *ifname, char *ifname_desc, unsigned long long *rx, unsigned long long *tx, char *ifname_desc2, unsigned long long *rx2, unsigned long long *tx2, char *nv_lan_ifname, char *nv_lan_ifnames)
{
	char word[100], word1[100], *next, *next1;
	char tmp[100];
	char modelvlan[32];
	int i, j, model, unit;
	const struct dummy_ifaces_s *p;
	char wl_ifnames[32] = { 0 };
#if defined(RTCONFIG_SWITCH_RTL8370M_PHY_QCA8033_X2) || \
    defined(RTCONFIG_SWITCH_RTL8370MB_PHY_QCA8033_X2)
	int wans = get_wans_dualwan();
#endif

	strcpy(ifname_desc2, "");

	model = get_model();
	bcmvlan_models(model, modelvlan);

	/* Exclude dummy interfaces. */
	for (p = &dummy_ifaces_list[0]; p->name != NULL; ++p) {
		if ((p->exact && !strcmp(p->name, ifname)) ||
		    (!p->exact && !strncmp(p->name, ifname, strlen(p->name))))
			return 0;
	}

#if defined(RTCONFIG_QCA)
#if defined(RTCONFIG_SWITCH_RTL8370M_PHY_QCA8033_X2) || \
    defined(RTCONFIG_SWITCH_RTL8370MB_PHY_QCA8033_X2)
	/* Handle LAN aggregation interfaces.
	 * tmcal.js converts LACPWx as WANx.
	 */
	if (nvram_match("lan_trunk_type", "2")
	 && ((wans & (WANSCAP_WAN | WANSCAP_WAN2 | WANSCAP_LAN)) == WANSCAP_LAN))
	{
		/* LAN as WAN, LAN LACP w/ two WAN ports. */
		int b;
		const char *q;
		uint32_t m = BS_WAN_PORT_MASK | BS_WAN2_PORT_MASK;

		while ((b = ffs(m)) > 0) {
			b--;
			if ((q = bs_port_id_to_iface(b)) == NULL) {
				m &= ~(1U << b);
				continue;
			}
			if (b == BS_WAN_PORT_ID && !strcmp(ifname, q)) {
				strlcpy(ifname_desc, "LACPW1", 12);
				return 1;
			}
			else if (b == BS_WAN2_PORT_ID && !strcmp(ifname, q)) {
				strlcpy(ifname_desc, "LACPW2", 12);
				return 1;
			}

			m &= ~(1U << b);
		}
	}
#endif

#if defined(RTCONFIG_LACP)
	/* Handle LAN aggregation interfaces.
	 * tmcal.js converts LACPx as LANx.
	 */
	if (nvram_match("lacp_enabled", "1")) {
		int b;
		const char *q;
#if defined(RTCONFIG_SWITCH_QCA8075_QCA8337_PHY_AQR107_AR8035_QCA8033)
		uint32_t m = BS_LAN1_PORT_MASK | BS_LAN2_PORT_MASK;
#else
#error	FIXME
#endif
		while ((b = ffs(m)) > 0) {
			b--;
			if (b >= BS_LAN1_PORT_ID && b <= BS_LAN8_PORT_ID
			 && (q = bs_port_id_to_iface(b)) != NULL && !strcmp(ifname, q)) {
				snprintf(ifname_desc, 12, "LACP%d", b);
				return 1;
			}

			m &= ~(1U << b);
		}
	}
#endif

#if defined(RTCONFIG_BONDING_WAN)
	/* Handle WAN aggregation interfaces.
	 * tmcal.js converts WAGGR? as correct port name.
	 */
	if (bond_wan_enabled()) {
		int b;
		const char *q;
		uint32_t m =  nums_str_to_u32_mask(nvram_get("wanports_bond"));

		while ((b = ffs(m)) > 0) {
			b--;
			if ((q = bs_port_id_to_iface(b)) != NULL && !strcmp(ifname, q)) {
				snprintf(ifname_desc, 12, "WAGGR%d", b);
				return 1;
			}

			m &= ~(1U << b);
		}
	}
#endif
#endif

	// find in LAN interface
	if (find_word(nv_lan_ifnames, ifname))
	{
		// find Wireless interface
		i=0;
		strlcpy(wl_ifnames, nvram_safe_get("wl_ifnames"), sizeof(wl_ifnames));
		foreach(word, wl_ifnames, next) {
			SKIP_ABSENT_BAND_AND_INC_UNIT(i);
			if(strcmp(word, ifname)==0) {
				sprintf(ifname_desc, "WIRELESS%d", i);
				return 1;
			}

#ifdef RTCONFIG_RALINK
			if(model == MODEL_RTN65U && strncmp(ifname, "rai", 3) == 0) // invalid traffic amount for interface
				return 0;
#endif

			snprintf(tmp, sizeof(tmp), "wl%d_vifnames", i);
			j = 0;
			foreach(word1, nvram_safe_get(tmp), next1) {
				if(strcmp(word1, wif_to_vif(ifname))==0)
				{
					sprintf(ifname_desc, "WIRELESS%d.%d", i, j);
					return 1;
				}
				j++;
			}

			i++;
		}

#if defined(RTCONFIG_RALINK) && defined(RTCONFIG_WLMODULE_RT3352_INIC_MII)
		if(model == MODEL_RTN65U)
		{
			i = 0;
			j = 0;
			foreach(word, nvram_safe_get("nic_lan_ifnames"), next) {
				if(strcmp(word, ifname)==0)
				{
					snprintf(tmp, sizeof(tmp), "wl%d_vifnames", 0);
					foreach(word1, nvram_safe_get(tmp), next1) {
						char vifname[32];
						snprintf(vifname, sizeof(vifname), "%s_ifname", word1);
						if(strlen(nvram_safe_get(vifname)) > 0)
						{
							if(i-- == 0)
								break;
						}
						j++;
					}
				sprintf(ifname_desc, "WIRELESS%d.%d", 0, j);
					return 1;
				}
				i++;
			}
		}
#endif

		// find wired interface
		strcpy(ifname_desc, "WIRED");

#if defined(RTAC53) //sherry fix rt-ac53 bug#5 traffic incorrect when hwnat enable{
		get_realtek_lans_bytecount(tx, rx);
#endif	//end sherry}	


		if(model == MODEL_DSLAC68U)
			return 1;

#ifdef RTCONFIG_BCM5301X_TRAFFIC_MONITOR
			return 1;
#endif

		// special handle for non-tag wan of broadcom solution
		// pretend vlanX is must called after ethX
		if(nvram_match("switch_wantag", "none")) { //Don't calc if select IPTV
			if(backup_set && strlen(modelvlan) && strcmp(ifname, modelvlan)==0) {
				backup_set  = 0;
				backup_rx -= *rx;
				backup_tx -= *tx;

				*rx2 = backup_rx;
				*tx2 = backup_tx;				
				/* Cherry Cho modified for RT-AC3200 Bug#202 in 2014/11/4. */
				unit = get_wan_unit(wan_if_eth());

				if ((unit == wan_primary_ifunit())
#ifdef RTCONFIG_DUALWAN
				    || (!strstr(nvram_safe_get("wans_dualwan"), "none") && nvram_match("wans_mode", "lb"))
#endif
				    ) {
					get_internet_desc(unit, ifname_desc2);
				}
			}
		}//End of switch_wantag

		return 1;
	}
	// find bridge interface
	else if(!strcmp(nv_lan_ifname, ifname))
	{
		strcpy(ifname_desc, "BRIDGE");
		return 1;
	}
	// find in WAN interface
	else if (ifname && (unit = get_wan_unit(ifname)) >= 0)	{
		if (dualwan_unit__nonusbif(unit)) {
#if defined(RA_ESW)
#if defined(RTCONFIG_RALINK_MT7620)
			get_mt7620_wan_unit_bytecount(unit, tx, rx);
#elif defined(RTCONFIG_RALINK_MT7621)
			get_mt7621_wan_unit_bytecount(unit, tx, rx);
#endif
#elif defined(RTAC53)//sherry fix rt-ac53 bug#5 traffic incorrect when hwnat enable{
			get_realtek_wan_bytecount(tx, rx);			
#endif//end sherry}
#if !defined(RTCONFIG_BCM5301X_TRAFFIC_MONITOR) && !defined(HND_ROUTER)
			if(strlen(modelvlan) && !strcmp(ifname, WAN_IF_ETH))
			{
				backup_rx = *rx;
				backup_tx = *tx;
				backup_set  = 1;
			}
			else
#endif
			{
				if ((unit == wan_primary_ifunit())
#ifdef RTCONFIG_DUALWAN
				    || (!strstr(nvram_safe_get("wans_dualwan"), "none") && nvram_match("wans_mode", "lb"))
#endif
				   ) {
					get_internet_desc(unit, ifname_desc);
					return 1;
				}
			}
		}
		else if (dualwan_unit__usbif(unit)) {
			if ((unit == wan_primary_ifunit())
#ifdef RTCONFIG_DUALWAN
			    || (!strstr(nvram_safe_get("wans_dualwan"), "none") && nvram_match("wans_mode", "lb"))
#endif
			   ) {
				get_internet_desc(unit, ifname_desc);
				return 1;
			}	
		}
		else {
			_dprintf("%s: unknown ifname %s\n", __func__, ifname);
		}
	}

	return 0;
}

/* 0: Not private subnet
 * 1: A class, 2: B class, 3: C class, 4: rfc6598 */
int is_private_subnet(const char *ip)
{
	const static struct {
		in_addr_t network;
		in_addr_t netmask;
	} classes[] = {
		{ __constant_htonl(0xc0a80000), __constant_htonl(0xffff0000) }, /* 192.168.0.0/16 */
		{ __constant_htonl(0xac100000), __constant_htonl(0xfff00000) }, /* 172.16.0.0/12 */
		{ __constant_htonl(0x0a000000), __constant_htonl(0xff000000) }, /* 10.0.0.0/8 */
		{ __constant_htonl(0x64400000), __constant_htonl(0xffc00000) }, /* 100.64.0.0/10 */
	};
	struct in_addr sin;
	int i;

	if (ip == NULL)
		return 0;

	if (inet_pton(AF_INET, ip, &sin) > 0) {
		for (i = 0; i < ARRAY_SIZE(classes); i++) {
			if ((sin.s_addr & classes[i].netmask) == classes[i].network)
				return i + 1;
		}
	}

	return 0;
}

// clean_mode: 0~3, clean_time: 0~(LONG_MAX-1), threshold(KB): 0: always act, >0: act when lower than.
int free_caches(const char *clean_mode, const int clean_time, const unsigned int threshold){
	int test_num;
	FILE *fp;
	char memdata[256] = {0};
	unsigned int memfree = 0;

#if defined(RTCONFIG_BCMARM) || defined(RTCONFIG_LANTIQ)
	return 0;
#endif

	/* Paul add 2012/7/17, skip free caches for DSL model. */
	#ifdef RTCONFIG_DSL
		return 0;
	#endif

	if(!clean_mode || clean_time < 0)
		return -1;

_dprintf("clean_mode(%s) clean_time(%d) threshold(%u)\n", clean_mode, clean_time, threshold);
	test_num = strtol(clean_mode, NULL, 10);
	if(test_num == LONG_MIN || test_num == LONG_MAX
			|| test_num < atoi(FREE_MEM_NONE) || test_num > atoi(FREE_MEM_ALL)
			)
		return -1;

	if(threshold > 0){
		if((fp = fopen("/proc/meminfo", "r")) != NULL){
			while(fgets(memdata, 255, fp) != NULL){
				if(strstr(memdata, "MemFree") != NULL){
					sscanf(memdata, "MemFree: %d kB", &memfree);
_dprintf("%s: memfree=%u.\n", __FUNCTION__, memfree);
					break;
				}
			}
			fclose(fp);
			if(memfree > threshold){
_dprintf("%s: memfree > threshold.\n", __FUNCTION__);
				return 0;
			}
		}
	}

_dprintf("%s: Start syncing...\n", __FUNCTION__);
	sync();

_dprintf("%s: Start cleaning...\n", __FUNCTION__);
	f_write_string("/proc/sys/vm/drop_caches", clean_mode, 0, 0);
	if(clean_time > 0){
_dprintf("%s: waiting %d second...\n", __FUNCTION__, clean_time);
		sleep(clean_time);
_dprintf("%s: Finish.\n", __FUNCTION__);
		f_write_string("/proc/sys/vm/drop_caches", FREE_MEM_NONE, FW_SILENT, 0);
	}

	return 0;
}

void logmessage_normal(char *logheader, char *fmt, ...){
  va_list args;
  char buf[512];
  char logheader2[33];
  int level;

  va_start(args, fmt);

  vsnprintf(buf, sizeof(buf), fmt, args);

  level = nvram_get_int("message_loglevel");
  if (level > 7) level = 7;

  strlcpy(logheader2, logheader, sizeof (logheader2));
  replace_char(logheader2, ' ', '_');
  openlog(logheader2, 0, 0);
  syslog(level, buf);
  closelog();
  va_end(args);
}

char *get_logfile_path(void)
{
	static char prefix[] = "/jffsXXXXXX";

#if defined(RTCONFIG_PSISTLOG)
	strlcpy(prefix, "/jffs", sizeof(prefix));
	if (!check_if_dir_writable(prefix)) {
		_dprintf("logfile output directory: /tmp.\n");
		strlcpy(prefix, "/tmp", sizeof(prefix));
	}
#else
	strlcpy(prefix, "/tmp", sizeof(prefix));
#endif

	return prefix;
}

char *get_syslog_fname(unsigned int idx)
{
	char prefix[] = "/jffsXXXXXX";
	static char buf[PATH_MAX];

#if defined(RTCONFIG_PSISTLOG)
	strlcpy(prefix, "/jffs", sizeof(prefix));
	if (!check_if_dir_writable(prefix)) {
		_dprintf("syslog output directory: /tmp.\n");
		strlcpy(prefix, "/tmp", sizeof(prefix));
	}
#else
	strlcpy(prefix, "/tmp", sizeof(prefix));
#endif

	if (!idx)
		snprintf(buf, sizeof(buf), "%s/syslog.log", prefix);
	else
		snprintf(buf, sizeof(buf), "%s/syslog.log-%d", prefix, idx);

	return buf;
}

#ifdef RTCONFIG_USB_MODEM
char *get_modemlog_fname(void){
	char prefix[] = "/tmpXXXXXX";
	static char buf[PATH_MAX];

	strlcpy(prefix, "/tmp", sizeof(prefix));
	snprintf(buf, sizeof(buf), "%s/3ginfo.txt", prefix);

	return buf;
}
#endif

#if defined(RTCONFIG_BCMWL6) && defined(RTCONFIG_PROXYSTA)
#ifdef RTCONFIG_DPSTA
int is_dpsta(int unit)
{
	char ifname[80], name[80], *next;
	int idx = 0;
	char wl_ifnames[32] = { 0 };

	if (dpsta_mode()) {
		strlcpy(wl_ifnames, nvram_safe_get("wl_ifnames"), sizeof(wl_ifnames));
		foreach (ifname, wl_ifnames, next) {
			if (idx == unit) break;
			idx++;
		}

		foreach(name, nvram_safe_get("dpsta_ifnames"), next) {
			if (!strcmp(name, ifname))
				return 1;
		}
	}

	return 0;
}
#endif

int is_dpsr(int unit)
{
	char ifname[32];

	if (dpsr_mode()) {
		if ((num_of_wl_if() == 2) || !unit || unit == nvram_get_int("dpsta_band"))
			return 1;

		if (strlen(nvram_safe_get("dpsr_ifnames"))) {
			snprintf(ifname, sizeof(ifname), "wl%d_ifname", unit);
			if (find_in_list(nvram_safe_get("dpsr_ifnames"), nvram_safe_get(ifname)))
				return 1;
		}
	}

	return 0;
}

#ifdef RTCONFIG_DPSR
int dpsr_main(char *nif)
{
	int nif_idx=0, idx=0, wlif=0;
	char wlc_valid = 0, valid_num = 0;
	char wl_ifnames[32] = { 0 };
	char ifname[32], *next;
	char wl_prefix[16], wlc_prefix[16], tmp[32];

	if (!dpsr_mode())
		return 0;

	if(nvram_match("x_Setting", "0")) {
		return 1;
	}

	strlcpy(wl_ifnames, nvram_safe_get("wl_ifnames"), sizeof(wl_ifnames));
	foreach (ifname, wl_ifnames, next) {
		if (!strcmp(nif, ifname)) {
			wlif = 1;
			break;
		}
		nif_idx++;
	}
	_dprintf("%s, nif-idx=%d, wlif=%d\n", __func__, nif_idx, wlif);

	if(!wlif) {
		return 1;
	}

	for(idx = 0; idx < num_of_wl_if(); ++idx) {
		snprintf(wl_prefix, sizeof(wl_prefix), "wl%d_", idx);
		snprintf(wlc_prefix, sizeof(wlc_prefix), "wlc%d_", idx);

		if(nvram_match(strlcat_r(wl_prefix, "mode", tmp, sizeof(tmp)), "wet") && *nvram_safe_get(strlcat_r(wlc_prefix, "ssid", tmp, sizeof(tmp)))) {
			wlc_valid |= 1<<idx;
			valid_num++;
		}
	}
	_dprintf("%s, wlc_valid=%d, num(%d)\n", __func__, wlc_valid, valid_num);

	if(!(wlc_valid & 1<<nif_idx)) {
		return 0;
	}
	
	if(valid_num == 1) {
		nvram_set("dpsr_main", nif);
		return 1;
	}
	
	if(nif_idx == 1) {
		nvram_set("dpsr_main", nif);
		return 1;
	} else {
		return 0;
	}
}
#endif

int is_psta(int unit)
{
#if defined(RTCONFIG_AMAS) && defined(CONFIG_BCMWL5)
       char tmp[128], word[128], *next;
       int idx = 0;
#endif

	if (unit < 0) return 0;
#if defined(RTCONFIG_AMAS) && defined(CONFIG_BCMWL5)
	if (sw_mode() == SW_MODE_ROUTER) {
		foreach (word, nvram_safe_get("wl_ifnames"), next) {
			snprintf(tmp, sizeof(tmp), "wl%d_nband", idx);
			if (nvram_get_int(tmp) == WLC_BAND_2G
			    && unit == idx
			    && !nvram_get_int("x_Setting")
			    && nvram_get_int("amesh_wps_enr"))
				return 1;
			idx++;
		}
	}
#endif
	if ((sw_mode() == SW_MODE_AP) &&
		(nvram_get_int("wlc_psta") == 1) &&
		(nvram_get_int("wlc_band") == unit))
		return 1;

#ifdef RPAX56
	if(nvram_match("use_dpsta", "1") && rp_mode())	// shall not use it
		return 1;
#endif
	return 0;
}

int is_psr(int unit)
{
	if (unit < 0) return 0;
#ifdef RTCONFIG_QTN
	if (unit == 1) return 0;
#endif
	if ((sw_mode() == SW_MODE_AP) &&
		(nvram_get_int("wlc_psta") == 2) &&
		(
		is_dpsr(unit) ||
#ifdef RTCONFIG_DPSTA
		is_dpsta(unit) || is_rp_unit(unit) ||
#ifdef RTCONFIG_AMAS
		dpsta_mode() ||
#endif
#endif
		((nvram_get_int("wlc_band") == unit) && !dpsr_mode()
#ifdef RTCONFIG_DPSTA
		&& !dpsta_mode()
#endif
		)
		))
		return 1;

	return 0;
}

int psta_exist(void)
{
	char word[256], *next;
	int idx = 0;
	char wl_ifnames[32] = { 0 };

	strlcpy(wl_ifnames, nvram_safe_get("wl_ifnames"), sizeof(wl_ifnames));
	foreach (word, wl_ifnames, next) {
		if (is_psta(idx)) return 1;
		idx++;
	}

	return 0;
}

int psta_exist_except(int unit)
{
	char word[256], *next;
	int idx = 0;
	char wl_ifnames[32] = { 0 };

	if (unit < 0) return 0;

	strlcpy(wl_ifnames, nvram_safe_get("wl_ifnames"), sizeof(wl_ifnames));
	foreach (word, wl_ifnames, next) {
		if (idx == unit) goto END;
		if (is_psta(idx)) return 1;
END:
		idx++;
	}

	return 0;
}

int psr_exist(void)
{
	char word[256], *next;
	int idx = 0;
	char wl_ifnames[32] = { 0 };

	strlcpy(wl_ifnames, nvram_safe_get("wl_ifnames"), sizeof(wl_ifnames));
	foreach (word, wl_ifnames, next) {
		if (is_psr(idx)) return 1;
		idx++;
	}

	return 0;
}

int psr_exist_except(int unit)
{
	char word[256], *next;
	int idx = 0;
	char wl_ifnames[32] = { 0 };

	if (unit < 0) return 0;

	strlcpy(wl_ifnames, nvram_safe_get("wl_ifnames"), sizeof(wl_ifnames));
	foreach (word, wl_ifnames, next) {
		if (idx == unit) goto END;
		if (is_psr(idx)) return 1;
END:
		idx++;
	}

	return 0;
}
#endif

#if defined(RTCONFIG_OPENVPN) || defined(RTCONFIG_IPSEC)
int set_crt_parsed(const char *name, char *file_path)
{
#if defined(RTCONFIG_JFFS2) || defined(RTCONFIG_BRCM_NAND_JFFS2) || defined(RTCONFIG_UBIFS)
	char target_file_path[128] ={0};

	if(!check_if_dir_exist(OVPN_FS_PATH))
		mkdir(OVPN_FS_PATH, S_IRWXU);

	if(check_if_file_exist(file_path)) {
		snprintf(target_file_path, sizeof(target_file_path) -1, "%s/%s", OVPN_FS_PATH, name);
		return eval("cp", file_path, target_file_path);
	}
	else {
		return -1;
	}
#else
	FILE *fp=fopen(file_path, "r");
	char buffer[8000] = {0};
	char buffer2[256] = {0};
	char *p = buffer;

	if(fp) {
		while(fgets(buffer, sizeof(buffer), fp)) {
			if(!strncmp(buffer, "-----BEGIN", 10))
				break;
		}
		if(feof(fp)) {
			fclose(fp);
			return -EINVAL;
		}
		p += strlen(buffer);
		//if( *(p-1) == '\n' )
			//*(p-1) = '>';
		while(fgets(buffer2, sizeof(buffer2), fp)) {
			strncpy(p, buffer2, strlen(buffer2));
			p += strlen(buffer2);
			//if( *(p-1) == '\n' )
				//*(p-1) = '>';
		}
		*p = '\0';
		nvram_set(name, buffer);
		fclose(fp);
		return 0;
	}
	else
		return -ENOENT;
#endif
}
#endif

#ifdef RTCONFIG_DUALWAN
/**
 * Get wan_unit of USB interface
 * @return
 *  >= 0:	wan_unit
 *  <  0:	not found
 */
int get_usbif_dualwan_unit(void)
{
	int wan_unit;

	for(wan_unit = WAN_UNIT_FIRST; wan_unit < WAN_UNIT_MAX; ++wan_unit)
		if(get_dualwan_by_unit(wan_unit) == WANS_DUALWAN_IF_USB)
			break;

	if(wan_unit == WAN_UNIT_MAX)
		wan_unit = -1;

	return wan_unit;
}

#ifdef RTCONFIG_USB_MULTIMODEM
int get_usb2if_dualwan_unit(void){
	int wan_unit;

	for(wan_unit = WAN_UNIT_FIRST; wan_unit < WAN_UNIT_MAX; ++wan_unit)
		if(dualwan_unit__usbif(wan_unit) == WANS_DUALWAN_IF_USB2)
			break;

	if(wan_unit == WAN_UNIT_MAX)
		wan_unit = -1;

	return wan_unit;
}
#endif

/**
 * Get unit of primary interface (WAN/LAN/USB only)
 * @return
 *  >= 0:	wan_unit
 *  <  0:	not found
 */
int get_primaryif_dualwan_unit(void)
{
	int unit, wan_type;

	for(unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; ++unit){
		if (unit != wan_primary_ifunit() && !nvram_match("wans_mode", "lb"))
			continue;

		wan_type = get_dualwan_by_unit(unit);
		if (!eth_wantype(unit) && wan_type != WANS_DUALWAN_IF_USB)
			continue;

		break;
	}

	if(unit == WAN_UNIT_MAX)
		unit = -1;

	return unit;
}
#endif

/**
 * Get wan unit of upstream port of IPTV.
 * Most models use WAN port as upstream port of IPTV.
 * If platform-specific API exist, it is in charge of return wan unit of upstream port of IPTV.
 * If platform-specific API doesn't exist, this function find WANS_DUALWAN_IF_WAN in all possible wan unit and return it.
 * @return:	wan unit that upstream port of IPTV.
 */
int get_upstream_wan_unit(void)
{
	int i, unit = -1;

	if (!__get_upstream_wan_unit) {
		for (i = WAN_UNIT_FIRST; unit < 0 && i < WAN_UNIT_MAX; ++i) {
			if (get_dualwan_by_unit(i) != WANS_DUALWAN_IF_WAN)
				continue;
			unit = i;
		}
	} else {
		unit = __get_upstream_wan_unit();
	}

	return unit;
}

/* Return WiFi unit number in accordance with interface name.
 * @wif:	pointer to WiFi interface name.
 * @return:
 * 	< 0:	invalid
 *  otherwise:	unit
 */
int get_wifi_unit(char *wif)
{
	int i;
	char word[256], *next, *ifn, nv[20];
	char wl_ifnames[32] = { 0 };

	if (!wif || *wif == '\0')
		return -1;

	strlcpy(wl_ifnames, nvram_safe_get("wl_ifnames"), sizeof(wl_ifnames));
	foreach (word, wl_ifnames, next) {
		SKIP_ABSENT_FAKE_IFACE(word);
		if (strncmp(word, wif, strlen(word)))
			continue;
#if defined(RTCONFIG_AMAS_WGN) && defined(RTCONFIG_QCA) 
		if (strlen(word)!=strlen(wif))
			continue;
#endif
		for (i = 0; i <= MAX_NR_WL_IF; ++i) {
			SKIP_ABSENT_BAND(i);
			snprintf(nv, sizeof(nv), "wl%d_ifname", i);
			ifn = nvram_get(nv);
			if (ifn != NULL && !strncmp(word, ifn, strlen(word)))
				return i;
		}
	}
	return -1;
}


#ifdef RTCONFIG_YANDEXDNS
int get_yandex_dns(int family, int mode, char **server, int max_count)
{
	static const struct {
		int mode;
		int family;
		char *server;
	} table[] = {
		{ YADNS_BASIC, AF_INET, "77.88.8.8" },			/* dns.yandex.ru  */
		{ YADNS_BASIC, AF_INET, "77.88.8.1" },			/* secondary.dns.yandex.ru */
		{ YADNS_SAFE, AF_INET, "77.88.8.88" },			/* safe.dns.yandex.ru */
		{ YADNS_SAFE, AF_INET, "77.88.8.2" },			/* secondary.safe.dns.yandex.ru */
		{ YADNS_FAMILY, AF_INET, "77.88.8.7" },			/* family.dns.yandex.ru */
		{ YADNS_FAMILY, AF_INET, "77.88.8.3" },			/* secondary.family.dns.yandex.ru */
#ifdef RTCONFIG_IPV6
		{ YADNS_BASIC, AF_INET6, "2a02:6b8::feed:0ff" },	/* dns.yandex.ru  */
		{ YADNS_BASIC, AF_INET6, "2a02:6b8:0:1::feed:0ff" },	/* secondary.dns.yandex.ru */
		{ YADNS_SAFE, AF_INET6, "2a02:6b8::feed:bad" },		/* safe.dns.yandex.ru */
		{ YADNS_SAFE, AF_INET6, "2a02:6b8:0:1::feed:bad" },	/* secondary.safe.dns.yandex.ru */
		{ YADNS_FAMILY, AF_INET6, "2a02:6b8::feed:a11" },	/* family.dns.yandex.ru */
		{ YADNS_FAMILY, AF_INET6, "2a02:6b8:0:1::feed:a11" },	/* secondary.family.dns.yandex.ru */
#endif
	};
	int i, count = 0;

	if (mode < YADNS_FIRST || mode >= YADNS_COUNT)
		return -1;

	for (i = 0; i < sizeof(table)/sizeof(table[0]) && count < max_count; i++) {
		if ((mode == table[i].mode) &&
		    (family == AF_UNSPEC || family == table[i].family))
			server[count++] = table[i].server;
	}

	return count;
}
#endif

int get_iface_hwaddr(char *name, unsigned char *hwaddr)
{
	struct ifreq ifr;
	int ret = 0;
	int s;

	/* open socket to kernel */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		return errno;
	}

	/* do it */
	strncpy(ifr.ifr_name, name, sizeof(ifr.ifr_name)-1);
	ifr.ifr_name[sizeof(ifr.ifr_name)-1] = '\0';
	if ((ret = ioctl(s, SIOCGIFHWADDR, &ifr)) == 0)
		memcpy(hwaddr, ifr.ifr_hwaddr.sa_data, ETHER_ADDR_LEN);

	/* cleanup */
	close(s);
	return ret;
}

#if defined(RTCONFIG_SOC_IPQ8064) || defined(RTCONFIG_SOC_IPQ8074)
/**
 * Set Receive/Transmit Packet Scaling of specified interface.
 * @ifname:	interface name
 * @nr_rx_mask:	number of elements in rx_mask array
 *    < 0:	Don't touch RPS of all RX queues of @ifname.
 *    = 0:	RPS of all RX queues are configured as rx_mask[0]
 *    > 0:	RPS of i-th RX queues is configured as rx_mask[i]
 * @rx_mask:	pointer to array, each element are CPU bit-mask, bit0 = CPU0, bit1 = CPU1, etc.
 * 		if rx_mask[i] equals to zero, RPS of i-th queue of @ifname is disabled.
 * @nr_tx_mask:	number of elements in tx_mask array
 *    < 0:	Don't touch XPS of all RX queues of @ifname.
 *    = 0:	XPS of all TX queues are configured as tx_mask[0]
 *    > 0:	XPS of i-th TX queues is configured as tx_mask[i]
 * @tx_mask:	pointer to array, each element are CPU bit-mask, bit0 = CPU0, bit1 = CPU1, etc.
 * 		if tx_mask[i] equals to zero, XPS of i-th queue of @ifname is disabled.
 * @return:
 * 	0:	success
 *     -1:	invalid parameter
 *     -2:	interface doesn't exist
 */
int __set_iface_ps(const char *ifname, int nr_rx_mask, const unsigned int *rx_mask, int nr_tx_mask, const unsigned int *tx_mask)
{
	const char *attr[] = { "rps_cpus", "xps_cpus" };
	int q, max_q = nr_rx_mask? nr_rx_mask : 32;
	char rx_m[16], tx_m[16];
	char prefix[sizeof("/sys/class/net/ethXXXXXX/queues")], path[sizeof("/sys/class/net/ethXXXXXX/queuese/tx-0/xps_cpusYYYYYY")];

	if (!ifname || *ifname == '\0' || nr_rx_mask > 32 || nr_tx_mask > 32)
		return -1;

	if ((nr_rx_mask >= 0 && !rx_mask) || (nr_tx_mask >= 0 && !tx_mask))
		return -1;

	if (!iface_exist(ifname))
		return -2;

	snprintf(prefix, sizeof(prefix), "%s/%s/queues", SYS_CLASS_NET, ifname);
	if (nr_rx_mask >= 0)
		snprintf(rx_m, sizeof(rx_m), "%x", rx_mask[0]);
	if (nr_tx_mask >= 0)
		snprintf(tx_m, sizeof(tx_m), "%x", tx_mask[0]);
	for (q = 0; q < max_q; ++q) {
		if (nr_rx_mask > 0)
			snprintf(rx_m, sizeof(rx_m), "%x", *(rx_m + q));
		if (nr_tx_mask > 0)
			snprintf(tx_m, sizeof(tx_m), "%x", *(tx_m + q));

		if (nr_rx_mask >= 0) {
			snprintf(path, sizeof(path), "%s/rx-%d/%s", prefix, q, attr[0]);
			f_write_string(path, rx_m, 0, 0);
		}
		if (nr_tx_mask >= 0) {
			snprintf(path, sizeof(path), "%s/tx-%d/%s", prefix, q, attr[1]);
			f_write_string(path, tx_m, 0, 0);
		}
	}

	return 0;
}

/**
 * Enable/disable GRO of specific interface via standard ethtool ioctl.
 * @iface:	interface name
 * @onoff:	ON/OFF GRO
 * @return:
 * 	 0:	success
 * 	-1:	invalid parameter
 * 	-2:	@iface doesn't exist
 * 	-3:	open socket error
 * 	-4:	ioctl error
 */
int ctrl_gro(char *iface, int onoff)
{
	/* ethtool-util.h */
#ifndef SIOCETHTOOL
#define SIOCETHTOOL	0x8946
#endif

	/* ethtool-copy.h */
#define ETHTOOL_GGRO		0x0000002b /* Get GRO enable (ethtool_value) */
#define ETHTOOL_SGRO		0x0000002c /* Set GRO enable (ethtool_value) */
	/* for passing single values */
	struct ethtool_value {
		__u32	cmd;
		__u32	data;
	};

	int fd, err;
	struct ifreq ifr;
	struct ethtool_value eval;

	if (!iface)
		return -1;

	if (!iface_exist(iface))
		return -2;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0)
		return -3;

	memset(&eval, 0, sizeof(eval));
	memset(&ifr, 0, sizeof(ifr));
	eval.cmd = ETHTOOL_SGRO;
	eval.data = !!onoff;
	ifr.ifr_data = (caddr_t) &eval;
	strlcpy(ifr.ifr_name, iface, sizeof(ifr.ifr_name));
	err = ioctl(fd, SIOCETHTOOL, &ifr);
	if (err) {
		dbg("Can't %s GRO on %s. errno %d (%s)\n",
			onoff? "enable" : "disable",
			iface, errno, strerror(errno));
	}

	close(fd);

	return err? -4 : 0;
}

/**
 * Enable/disable GRO on specific WAN unit.
 * Only WAN, WAN2, LAN as WAN are supported, skip DSL, USB.
 * @wan_unit:
 * @onoff:
 * @return:
 * 	 0:	success
 * 	-1:	invalid wan_unit
 * 	-2:	not supported WAN type
 * 	-3:	Can't enable GRO on WAN interface.
 * 	-4:	ctrl_gro() fail
 */
int ctrl_wan_gro(int wan_unit, int onoff)
{
	int r;
	char prefix[8];

	if (wan_unit < WAN_UNIT_FIRST || wan_unit >= WAN_UNIT_MAX)
		return -1;

	if (!eth_wantype(wan_unit))
		return -2;

	snprintf(prefix, sizeof(prefix), "wan%d_", wan_unit);
	if (!strncmp(nvram_pf_safe_get(prefix, "ifname"), "vlan", 4))
		return -2;

	if (onoff)
		return -3;

	r = ctrl_gro(nvram_pf_safe_get(prefix, "ifname"), onoff);

	return r? -4 : 0;
}

/**
 * Enable/disable GRO on all interfaces in lan_ifnames nvram variable.
 * e.g. LAN/bridge/bonding interfaces.
 * @wan_unit:
 * @onoff:
 * @return:
 * 	 0:	success
 * 	-1:	one or more ctrl_gro() fail
 */
int ctrl_lan_gro(int onoff)
{
	DIR *dir;
	int c = 0, r;
	struct dirent *entry;
	char word[64], *next, iface[IFNAMSIZ], ifnames[256];
	char path[sizeof("/sys/class/net/bondXXXXXX")];

	strlcpy(ifnames, nvram_safe_get("lan_ifname"), sizeof(ifnames));
	strlcat(ifnames, " ", sizeof(ifnames));
	strlcat(ifnames, nvram_safe_get("lan_ifnames"), sizeof(ifnames));
	foreach(word, ifnames, next) {
		SKIP_ABSENT_FAKE_IFACE(word);
		/* Skip VLAN, TUN, TAP, and Wireless interface. */
		if (!strncmp(word, "vlan", 4) || !strncmp(word, "tun", 3) || !strncmp(word, "tap", 3)
#if defined(RTCONFIG_RALINK)
		    || !strncmp(word, "ra", 2)
#elif defined(RTCONFIG_QCA)
		    || !strncmp(word, "ath", 3)
#if defined(RTCONFIG_WIGIG)
		    || !strncmp(word, "wlan", 4)
#endif
#else
		    || !strncmp(word, "wl", 2)
#endif
		   )
			continue;

		if (!iface_exist(word))
			continue;
		r = ctrl_gro(word, onoff);
		if (r < 0)
			c++;

		if (!strncmp(word, "bond", 4)) {
			/* Enable/disable GRO of all slaves interface of the bonding interface. */
			if ((dir = opendir(path)) == NULL)
				continue;

			while ((entry = readdir(dir)) != NULL) {
				if (strncmp(entry->d_name, "slave_", 6))
					continue;
				strlcpy(iface, entry->d_name + 6, sizeof(iface));
				r = ctrl_gro(iface, onoff);
				if (r < 0)
					c++;
			}
			closedir(dir);
		}
	}

	return c? -1 : 0;
}
#endif

/**
 * Read unsigned long variable from memory.
 * @addr:	address
 * @value:	pointer to a unsigned long integer variable
 * @return:
 * 	0:	success
 *     -1:	invalid parameter.
 *     -2:	open /dev/mem failed
 */
int u_readl(unsigned long addr, unsigned long *value)
{
	off_t offset;
	int ret, page_size;
	char *map;
	static int kfd = -1;
	unsigned long *p4;

	if (!value)
		return 0;

	page_size = getpagesize();
	offset = addr & ~(page_size - 1);

	kfd = open("/dev/mem", O_RDONLY);
	if (kfd < 0) {
		dbg("%s: open /dev/mem for accessing %x failed, errno %d (%s).,",
			__func__, addr, errno, strerror(errno));
		return -2;
	}

	map = mmap(NULL, page_size, PROT_READ, MAP_SHARED, kfd, offset);
	if (map == MAP_FAILED) {
		dbg("%s: mmap for address %x failed, errno %d (%s).\n",
			__func__, addr, errno, strerror(errno));
		close(kfd);
		return -3;
	}

	p4 = (unsigned long*) (map + (addr - offset));
	*value = *p4;

	ret = munmap(map, page_size);
	if (ret) {
		dbg("%s: munmap() return %d errno %d (%s)\n",
			__func__, ret, errno, strerror(errno));
		close(kfd);
		return -4;
	}

	close(kfd);

	return 0;
}

/**
 * Set smp_affinity of specified irq
 * @irq:
 * @cpu_mask:
 * @return:
 * 	0:	success
 *     -1:	irq not exist
 */
int set_irq_smp_affinity(unsigned int irq, unsigned int cpu_mask)
{
	char mask[16], path[sizeof("/proc/irq/XXXXXX/smp_affinityYYYYYY")];

	snprintf(path, sizeof(path), "%s/%d", PROC_IRQ, irq);
	if (!d_exists(path))
		return -1;

	snprintf(path, sizeof(path), "%s/%d/smp_affinity", PROC_IRQ, irq);
	snprintf(mask, sizeof(mask), "%x", cpu_mask);
	f_write_string(path, mask, 0, 0);

	return 0;
}

#if defined(RTCONFIG_SOC_IPQ8074) || defined(RTCONFIG_SOC_IPQ60XX)
#define PROC_INTR_FMT	"%d: %*d %*d %*d %*d %*s %*d %*s %64s"		/* kernel 4.4.x, 4 cores */
#elif defined(RTCONFIG_SOC_IPQ50XX)
#define PROC_INTR_FMT	"%d: %*d %*d %*s %*d %*s %64s"			/* kernel 4.4.x, 2 cores */
#elif defined(RTCONFIG_SOC_IPQ8064)
#define PROC_INTR_FMT	"%d: %*d %*d %*s %64s"				/* kernel 3.4.x, 2 cores */
#elif defined(RTCONFIG_SOC_IPQ40XX)
#define PROC_INTR_FMT	"%d: %*d %*d %*d %*d %*s %*d %64s"		/* kernel 3.14.x, 4 cores */
#else
#define PROC_INTR_FMT	"%d: %*d %*s %*s %64s"				/* kernel 2.6.36.x, 1 core */
#endif

/**
 * Set smp_affinity of specified irq by name
 * @name:	last field of /proc/interrupts
 * @order:
 * 	<= 0:	set smp_affinity of all irq with same @name
 *  otherwise:	set smp_affinity of @order-th irq with same @name only
 * @cpu_mask:
 * @return:
 * 	0:	success
 *     -1:	invalid parameter
 *     -2:	can't open /proc/interrupts
 */
int set_irq_smp_affinity_by_name(const char *name, int order, unsigned int cpu_mask)
{
	int irq, ord = 0;
	FILE *fp;
	char line[256], irq_name[64];
	char mask[16], path[sizeof("/proc/irq/XXXXXX/smp_affinityYYYYYY")];

	if (!name || *name == '\0')
		return -1;

	if (!(fp = fopen("/proc/interrupts", "r")))
		return -2;

	/* kernel 4.4.60 /proc/interrupts example.
	 *            CPU0       CPU1       CPU2       CPU3
	 *  18:    1019546    1126047     446170     840983       GIC  20 Edge      arch_timer
	 *  21:          0          0          0          0       GIC 270 Level     bam_dma
	 *  33:          0          0          0          0       GIC 411 Edge      nss-tx-unblock
	 *  34:    2805573          0          0          0       GIC 412 Edge      nss_queue0
	 *  35:          0          0          0          0       GIC 413 Edge      nss_queue1
	 *  36:          0          0          0          0       GIC 414 Edge      nss_queue2
	 *  37:          0          0          0          0       GIC 415 Edge      nss_queue3
	 *  38:          0          0          0          0       GIC 416 Edge      nss_coredump_complete
	 *  43:      58886          0          0          0       GIC 425 Edge      nss_queue0
	 *  44:          0          0          0          0       GIC 426 Edge      nss_queue1
	 *  45:          0          0          0          0       GIC 427 Edge      nss_queue2
	 *  46:          0          0          0          0       GIC 428 Edge      nss_queue3
	 * 106:       1617          0          0          0       GIC 189 Edge      wbm2host-tx-completions-ring1
	 * 110:          0          0          0          0       GIC 216 Edge      tsens_interrupt
	 * 217:          0          0          0          0   PCI-MSI   0 Edge      PCIe PME
	 * 250:          0          0          1          0     smp2p   1 Edge      error_ready_interrupt
	 * 253:          0          0          0          0       GIC 172 Edge      xhci-hcd:usb1
	 * IPI0:          0          0          0          0  CPU wakeup interrupts
	 * Err:          0
	 */
	fgets(line, sizeof(line), fp); //skip the 1st line
	while (fgets(line, sizeof(line), fp)) {
		if (*line == '\0' || !strncmp(line, "IPI", 3) || !strncmp(line, "Err", 3) || !strncmp(line, "ERR", 3))
			continue;

		if (sscanf(line, PROC_INTR_FMT, &irq, irq_name) != 2) {
			//dbg("%s: can't parse line [%s]\n", __func__, line);
			continue;
		}

		if (strcmp(irq_name, name))
			continue;

		ord++;

		if (order > 0 && ord != order)
			continue;

		snprintf(path, sizeof(path), "%s/%d", PROC_IRQ, irq);
		if (!d_exists(path))
			continue;

		snprintf(path, sizeof(path), "%s/%d/smp_affinity", PROC_IRQ, irq);
		snprintf(mask, sizeof(mask), "%x", cpu_mask);
		f_write_string(path, mask, 0, 0);

		/* If @order is specified and we just configure it, stop while loop. */
		if (order > 0 && ord == order)
			break;
	}

	fclose(fp);

	return 0;
}

/**
 * Get active firmware number.
 * @return:
 * 	0:	DUT boot from 1st firmware.
 * 	1:	DUT boot from 2nd firmware.
 */
int get_active_fw_num(void)
{
#if defined(RTCONFIG_DUAL_TRX2)
	int ret = 0, idx = -1;
	char name[sizeof("mtdblockXYYYYYY")];
	char path[sizeof("/sys/class/mtd/mtdXYYY/nameYYYYYY")];
	struct mntent *mnt;
	struct stat sb;

	mnt = findmntents("/", 0, NULL, 0);
	if (!mnt || lstat(mnt->mnt_fsname, &sb) != 0)
		return ret;

	if ((sb.st_mode & S_IFMT) == S_IFLNK) {
		memset(name, 0, sizeof(name));
		if (readlink(mnt->mnt_fsname, name, sizeof(name) - 1) <= 0) {
			dbg("%s: can't read symlink of %s\n", __func__, mnt->mnt_fsname);
			return ret;
		}
	} else {
		strlcpy(name, mnt->mnt_fsname, sizeof(name));
	}

	if (!strncmp(name, "mtdblock", 8)) {
		idx = safe_atoi(name + 8);
	} else if (!strncmp(name, "mtd", 3)) {
		idx = safe_atoi(name + 3);
	} else {
		dbg("%s: %s is not MTD.\n", __func__, mnt->mnt_fsname);
		return ret;
	}
	snprintf(path, sizeof(path), "%s/mtd%d/name", SYS_CLASS_MTD, idx);
	if (f_read_string(path, name, sizeof(name)) <= 0)
		return ret;
	if (!strncmp(name, "rootfs2", strlen("rootfs2")))
		ret = 1;

	return ret;
#else
	return 0;
#endif
}

/**
 * Execute @cmd, find @keyword in output and parse it.
 * @cmd:
 * @keyword:	If specified, only parse line with it.
 * @fmt:	sscanf format string
 * @cnt:	number of parameters should be get by sscanf()
 * @return:
 * 	0:	success
 *  otherwise:	fail
 */
int exec_and_parse(const char *cmd, const char *keyword, const char *fmt, int cnt, ...)
{
	int r, ret = 1;
	char line[256];
	FILE *fp;
	va_list args;

	if (!cmd || cnt <= 0 || !fmt)
		return -1;

	if (!(fp = popen(cmd, "r"))) {
		dbg("%s: can't execute [%s]\n", __func__, cmd);
		return -2;
	}

	va_start(args, cnt);
	while (ret && fgets(line, sizeof(line), fp)) {
		if (keyword && !strstr(line, keyword))
			continue;
		if ((r = vsscanf(line, fmt, args)) != cnt) {
			dbg("%s: Unknown output: [%s] of cmd [%s], fmt [%s], cnt %d, r %d\n",
				__func__, line, cmd, fmt, cnt, r);
			continue;
		}
		ret = 0;
	}
	va_end(args);
	pclose(fp);

	return ret;
}

/**
 * Run "iwpriv XXX get_XXX" and return string behind colon.
 * Expected result is shown below:
 * ath1      get_mode:11ACVHT40
 *                    ^^^^^^^^^ result
 * @iface:	interface name
 * @cmd:	get cmd
 * @return:
 * 	NULL	invalid parameter or error.
 *  otherwise:	success
 */
char *iwpriv_get(const char *iface, char *cmd)
{
	char iwpriv_cmd[sizeof("iwpriv athX CCCCCCCCCCCCCCCCXXX") + IFNAMSIZ];
	static char result[256] = { 0 };

	if (!iface || !cmd)
		return NULL;

	snprintf(iwpriv_cmd, sizeof(iwpriv_cmd), "iwpriv %s %s", iface, cmd);
	if (exec_and_parse(iwpriv_cmd, iface, "%*[^:]:%256[^\n]", 1, result))
		return NULL;

	return result;
}

/**
 * Run "iwpriv XXX get_XXX" and return integer behind colon.
 * Expected result is shown below:
 * ath1      channf:-90
 *                  ^^^ result
 * @iface:	interface name
 * @cmd:	get cmd
 * @return:
 * 	0:	success
 *     -1:	invalid parameter;
 *     -2:	error
 */
int iwpriv_get_int(const char *iface, char *cmd, int *result)
{
	char iwpriv_cmd[sizeof("iwpriv athX CCCCCCCCCCCCCCCCXXX") + IFNAMSIZ];

	if (!iface || !cmd || !result)
		return -1;

	snprintf(iwpriv_cmd, sizeof(iwpriv_cmd), "iwpriv %s %s", iface, cmd);
	if (exec_and_parse(iwpriv_cmd, iface, "%*[^:]:%d", 1, result))
		return -2;

	return 0;
}

/**
 * readdir() wrapper
 * This function is used to simplify opendir()/readdir()/closedir().
 * If @keyword is NULL or empty string, @handler is called for all items under @path.
 * If @keyword is a string, @handler is called only @keyword can be found in items under @path.
 * @path:	path to a directory.
 * @keyword:	used to filter specific items to @handler.
 * @handler:	function pointer.
 * @arg:	parameter for @handler.
 * @return:
 * 	0:	success
 *     -1:	invalid parameter
 *     -2:	can't open @path
 *     -3:	error reported by @handler
 */
int readdir_wrapper(const char *path, const char *keyword, int (*handler)(const char *path, const struct dirent *de, void *arg), void *arg)
{
	int ret = 0;
	DIR *dir;
	struct dirent *de;

	if (!path || !handler)
		return -1;

	if (!(dir = opendir(path)))
		return - 2;

	while ((de = readdir(dir)) != NULL) {
		if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, ".."))
			continue;
		if (!keyword || *keyword == '\0' || strstr(de->d_name, keyword) != NULL) {
			if (handler(path, de, arg))
				ret = -3;
		}
	}

	closedir(dir);

	return ret;
}

/**
 * Get prefix for QoS related nvram variables
 * If RTCONFIG_MULTIWAN_CFG is enabled, this function writes "qosX_" to @buf or internal buffer.
 * If RTCONFIG_MULTIWAN_CFG is not enabled, this function always writes "qos_" to @buf or internal buffer.
 * @unit:	WAN unit
 * @buf:	external buffer
 * 		If @buf is NULL, internal buffer would be used.
 * 		But it is not reentry-safe.
 * @return:	pointer to prefix.
 * 		If buf is not NULL, @return would be @buf.
 */
char *get_qos_prefix(int unit, char *buf)
{
	static char internal_buffer[32];	/* qosX_ */
	char *r = internal_buffer;

	if (buf)
		r = buf;

	strcpy(r, "qos_");

	return r;
}

int internet_ready(void)
{
	if(nvram_get_int("ntp_ready") == 1)
		return 1;
	return 0;
}

void set_no_internet_ready(void)
{
	nvram_set("ntp_ready", "0");
}

unsigned int num_of_mssid_support(unsigned int unit)
{
	char tmp_vifnames[] = "wlx_vifnames";
	char word[256], *next;
	int subunit;
#if defined(RTCONFIG_RALINK) || defined(RTCONFIG_QCA)
	if (nvram_match("wl_mssid", "0"))
		return 0;
#endif
	snprintf(tmp_vifnames, sizeof(tmp_vifnames), "wl%d_vifnames", unit);

	subunit = 0;
	foreach (word, nvram_safe_get(tmp_vifnames), next) {
		subunit++;
	}
	return subunit;
}

/**
 * Get led_id enumeration via wireless band.
 * @band:	0 ~ MAX_NR_WL_IF - 1
 * @return:	led_id enumeration.
 *  LED_ID_MAX:	led_id for @band is not found.
 */
enum led_id get_wl_led_id(int band)
{
	enum led_id led = LED_ID_MAX;

	if (band < 0 || band >= MAX_NR_WL_IF)
		return led;

	switch (band) {
	case WL_2G_BAND:
#if defined(PLN12)
		led = LED_2G_RED;
#elif defined(PLAC56)
		led = LED_2G_GREEN;
#else
		led = LED_2G;
#endif
		break;
#if defined(RTCONFIG_HAS_5G)
	case WL_5G_BAND:
#if defined(PLAC56)
		led = LED_5G_GREEN;
#else
		led = LED_5G;
#endif
		break;
#if defined(RTCONFIG_HAS_5G_2)
	case WL_5G_2_BAND:
		led = LED_5G2;
		break;
#endif
#endif
#if defined(RTCONFIG_WIGIG)
	case WL_60G_BAND:
		led = LED_60G;
		break;
#endif
	default:
		dbg("%s: Unknown wl%d band!\n", __func__, band);
	}

	return led;
}

/**
 * Get led gpio nvram variable name via wireless band.
 * @band:	0 ~ MAX_NR_WL_IF - 1
 * @return:	char pointer to a nvram variable name, e.g., led_2g_gpio.
 *  empty string:	led gpio nvram variable for @band is not found.
 */
char *get_wl_led_gpio_nv(int band)
{
	char *gpio_nv = "";

	if (band < 0 || band >= MAX_NR_WL_IF)
		return gpio_nv;

	switch (band) {
	case WL_2G_BAND:
		gpio_nv = "led_2g_gpio";
		break;
#if defined(RTCONFIG_HAS_5G)
	case WL_5G_BAND:
		gpio_nv = "led_5g_gpio";
		break;
#if defined(RTCONFIG_HAS_5G_2)
	case WL_5G_2_BAND:
		gpio_nv = "led_5g2_gpio";
		break;
#endif
#endif
#if defined(RTCONFIG_WIGIG)
	case WL_60G_BAND:
		gpio_nv = "led_60g_gpio";
		break;
#endif
	default:
		dbg("%s: Unknown wl%d band!\n", __func__, band);
	}

	return gpio_nv;
}

/* Convert 2G @ch to a bit-number of 64-bit mask.
 * @ch:	legal 2G channel
 * @return:
 * 	>= 0:	bit-number of @ch. ch1 ~ ch13 use bit 0~12
 * 	<  0:	invalid @ch
 */
int ch2g2bit(int ch)
{
	int b = -1;

	if (ch >= 1 && ch <= 13) {
		b = ch - 1;
	}

	return b;
}

/* Convert 2G @ch to a 64-bit mask.
 * @ch:	legal 2G channel
 * @return:
 * 	> 0:	bit mask of @ch. ch1 ~ ch13 use bit 0~12
 * 	= 0:	invalid @ch
 */
uint64_t ch2g2bitmask(int ch)
{
	int b = ch2g2bit(ch);

	return (b >= 0)? 1L << b : 0;
}

/* Convert @bit to 2G channel.
 * @bit:	bit number. (0~12 = ch1 ~ ch13)
 * @return:
 * 	>  0:	2G channel number of @bit
 * 	<= 0:	invalid @bit
 */
int bit2ch2g(int bit)
{
	int ch = 0;

	if (bit >= 0 && bit <= 12) {
		/* ch1 ~ ch13 */
		ch = bit + 1;
	}

	return ch;
}

/* Convert 2G channel list @ch_list that is seperate by @sep to bit-mask.
 * @ch_list:	channel list string, seperated by @sep
 * @sep:	character that is used to seperate channel
 * @return:
 * 	0:	error or none of any legal 2G channel in @ch_list
 *  otherwise:	bit-mask of @ch_list
 */
uint64_t chlist2g2bitmask(char *ch_list, char *sep)
{
	uint64_t m = 0;
	char ch[4], *next;
	int bit;

	if (!ch_list || !sep || strlen(sep) > 1)
		return 0;

	__foreach (ch, ch_list, next, sep) {
		if ((bit = ch2g2bit(safe_atoi(ch))) < 0)
			continue;

		m |= (1L << bit);
	}
	return m;
}

/* Convert bit-mask to 2G channel list that is seperated by @sep.
 * @mask:	bit-mask of 2G channels
 * @sep:	seperate character
 * @ch_list:	channel list buffer
 * @ch_list_len: length of @ch_list
 * @return:	pointer to converted string.
 */
char *__bitmask2chlist2g(uint64_t mask, char *sep, char *ch_list, size_t ch_list_len)
{
	int b, c;
	char ch[4];
	uint64_t m = mask;

	if (!ch_list || !ch_list_len || !sep || strlen(sep) > 1)
		return "";

	*ch_list = '\0';
	while ((b = ffsl(m)) > 0) {
		b--;
		if ((c = bit2ch2g(b)) <= 0)
			continue;
		snprintf(ch, sizeof(ch), "%d", bit2ch2g(b));
		if (*ch_list != '\0')
			strlcat(ch_list, sep, ch_list_len);
		strlcat(ch_list, ch, ch_list_len);

		m &= ~(1L << b);
	}

	return ch_list;
}

/* Convert bit-mask to 2G channel list that is seperated by @sep.
 * @mask:	bit-mask of 2G channels
 * @return:	pointer to converted string.
 */
char *bitmask2chlist2g(uint64_t mask, char *sep)
{
	static char ch_list[9 * 3 + 4 * 3 + 4];

	return __bitmask2chlist2g(mask, sep, ch_list, sizeof(ch_list));
}

/* Convert 5G @ch to a bit-number of 64-bit mask.
 * @ch:	legal 5G channel
 * @return:
 * 	>= 0:	bit-number of @ch. ch32 ~ ch68 use bit 0~9, ch96 ~ ch177 use bit 10~30.
 * 	<  0:	invalid @ch
 */
int ch5g2bit(int ch)
{
	int b = -1;

	if (ch >= 32 && ch <= 144) {
		if (!(ch & 3U)) {
			if (ch <= 68)
				b = ((ch - 32) >> 2);
			else
				b = ((ch - 32 - 24) >> 2);
		}
	}
	else if (ch >= 149 && ch <= 177) {
		if (!((ch - 1) & 3U)) {
			b = ((ch - 33 - 24) >> 2);
		}
	}

	return b;
}

/* Convert 5G @ch to a 64-bit mask.
 * @ch:	legal 5G channel
 * @return:
 * 	> 0:	bit mask of @ch. ch32 ~ ch68 use bit 0~9, ch96 ~ ch177 use bit 10~30.
 * 	= 0:	invalid @ch
 */
uint64_t ch5g2bitmask(int ch)
{
	int b = ch5g2bit(ch);

	return (b >= 0)? 1L << b : 0;
}

/* Convert @bit to 5G channel.
 * @bit:	bit number. (0~9 = ch32 ~ ch68, 10 ~ 30 = ch96 ~ ch177)
 * @return:
 * 	>  0:	5G channel number of @bit
 * 	<= 0:	invalid @bit
 */
int bit2ch5g(int bit)
{
	int ch = 0;

	if (bit >= 0 && bit <= 9) {
		/* ch32 ~ ch68 */
		ch = (bit << 2) + 32;
	} else if (bit >= 10 && bit <= 22) {
		/* ch96 ~ ch144 */
		ch = (bit << 2) + 32 + 24;
	} else if (bit >= 23 && bit <= 30) {
		/* ch149 ~ ch177 */
		ch = (bit << 2) + 33 + 24;
	}

	return ch;
}

/* Convert 5G channel list @ch_list that is seperate by @sep to bit-mask.
 * @ch_list:	channel list string, seperated by @sep
 * @sep:	character that is used to seperate channel
 * @return:
 * 	0:	error or none of any legal 5G channel in @ch_list
 *  otherwise:	bit-mask of @ch_list
 */
uint64_t chlist5g2bitmask(char *ch_list, char *sep)
{
	uint64_t m = 0;
	char ch[4], *next;
	int bit;

	if (!ch_list || !sep || strlen(sep) > 1)
		return 0;

	__foreach (ch, ch_list, next, sep) {
		if ((bit = ch5g2bit(safe_atoi(ch))) < 0)
			continue;

		m |= (1L << bit);
	}
	return m;
}

/* Convert bit-mask to 5G channel list that is seperated by @sep.
 * @mask:	bit-mask of 5G channels
 * @ch_list:	channel list buffer
 * @ch_list_len: length of @ch_list
 * @return:	pointer to converted string.
 */
char *__bitmask2chlist5g(uint64_t mask, char *sep, char *ch_list, size_t ch_list_len)
{
	int b, c;
	char ch[4];
	uint64_t m = mask;

	if (!ch_list || !ch_list_len || !sep || strlen(sep) > 1)
		return "";

	*ch_list = '\0';
	while ((b = ffsl(m)) > 0) {
		b--;
		if ((c = bit2ch5g(b)) <= 0)
			continue;
		snprintf(ch, sizeof(ch), "%d", bit2ch5g(b));
		if (*ch_list != '\0')
			strlcat(ch_list, sep, ch_list_len);
		strlcat(ch_list, ch, ch_list_len);

		m &= ~(1L << b);
	}

	return ch_list;
}

/* Convert bit-mask to 5G channel list that is seperated by @sep.
 * @mask:	bit-mask of 5G channels
 * @return:	pointer to converted string.
 */
char *bitmask2chlist5g(uint64_t mask, char *sep)
{
	static char ch_list[11 * 3 + 20 * 4 + 4];

	return __bitmask2chlist5g(mask, sep, ch_list, sizeof(ch_list));
}

#if defined(RTCONFIG_WIFI6E)
/* Convert 6G @ch to a bit-number of 64-bit mask.
 * @ch:	legal 6G channel
 * @return:
 * 	>= 0:	bit-number of @ch. ch33 ~ ch233 use bit 0~50
 * 	<  0:	invalid @ch
 */
int ch6g2bit(int ch)
{
	int b = -1;

	if (ch >= 33 && ch <= 233) {
		if (!((ch - 1) & 3U))
			b = ((ch - 33) >> 2);
	}

	return b;
}

/* Convert 6G @ch to a 64-bit mask.
 * @ch:	legal 6G channel
 * @return:
 * 	> 0:	bit mask of @ch. ch33 ~ ch233 use bit 0~50
 * 	= 0:	invalid @ch
 */
uint64_t ch6g2bitmask(int ch)
{
	int b = ch6g2bit(ch);

	return (b >= 0)? 1L << b : 0;
}

/* Convert @bit to 6G channel.
 * @bit:	bit number. (0~50 = ch33 ~ ch233)
 * @return:
 * 	>  0:	6G channel number of @bit
 * 	<= 0:	invalid @bit
 */
int bit2ch6g(int bit)
{
	int ch = 0;

	if (bit >= 0 && bit <= 50) {
		/* ch33 ~ ch233 */
		ch = (bit << 2) + 33;
	}

	return ch;
}

/* Convert 6G channel list @ch_list that is seperate by @sep to bit-mask.
 * @ch_list:	channel list string, seperated by @sep
 * @sep:	character that is used to seperate channel
 * @return:
 * 	0:	error or none of any legal 6G channel in @ch_list
 *  otherwise:	bit-mask of @ch_list
 */
uint64_t chlist6g2bitmask(char *ch_list, char *sep)
{
	uint64_t m = 0;
	char ch[4], *next;
	int bit;

	if (!ch_list || !sep || strlen(sep) > 1)
		return 0;

	__foreach (ch, ch_list, next, sep) {
		if ((bit = ch6g2bit(safe_atoi(ch))) < 0)
			continue;

		m |= (1L << bit);
	}
	return m;
}

/* Convert bit-mask to 6G channel list that is seperated by @sep.
 * @mask:	bit-mask of 6G channels
 * @ch_list:	channel list buffer
 * @ch_list_len: length of @ch_list
 * @return:	pointer to converted string.
 */
char *__bitmask2chlist6g(uint64_t mask, char *sep, char *ch_list, size_t ch_list_len)
{
	int b, c;
	char ch[4];
	uint64_t m = mask;

	if (!ch_list || !ch_list_len || !sep || strlen(sep) > 1)
		return "";

	*ch_list = '\0';
	while ((b = ffsl(m)) > 0) {
		b--;
		if ((c = bit2ch6g(b)) <= 0)
			continue;
		snprintf(ch, sizeof(ch), "%d", bit2ch6g(b));
		if (*ch_list != '\0')
			strlcat(ch_list, sep, ch_list_len);
		strlcat(ch_list, ch, ch_list_len);

		m &= ~(1L << b);
	}

	return ch_list;
}

/* Convert bit-mask to 6G channel list that is seperated by @sep.
 * @mask:	bit-mask of 6G channels
 * @return:	pointer to converted string.
 */
char *bitmask2chlist6g(uint64_t mask, char *sep)
{
	static char ch_list[17 * 3 + 34 * 4 + 4];

	return __bitmask2chlist6g(mask, sep, ch_list, sizeof(ch_list));
}
#endif

/* Convert @ch to a bit-number of 64-bit mask.
 * @band:	specific wireless band
 * @ch:		legal channel of @band
 * @return:
 * 	>= 0:	bit-number of @ch. (2G: ch1 ~ ch13 use bit 0~12
 * 				    5G: ch32 ~ ch68 use bit 0~9, ch96 ~ ch177 use bit 10~30
 * 				    6G: ch33 ~ ch233 use bit 0~50)
 * 	<  0:	invalid @ch
 */
int ch2bit(enum wl_band_id band, int ch)
{
	if (band < 0 || band >= WL_NR_BANDS)
		return -1;
#if defined(RTCONFIG_WIFI6E)
	else if (is_6g(band))
		return ch6g2bit(ch);
#endif
	else if (band == WL_5G_BAND || band == WL_5G_2_BAND)
		return ch5g2bit(ch);
	else
		return ch2g2bit(ch);
}

/* Convert @ch to a 64-bit mask.
 * @band:	specific wireless band
 * @ch:		legal channel of @band
 * @return:
 * 	> 0:	bit-number of @ch. (2G: ch1 ~ ch13 use bit 0~12
 * 				    5G: ch32 ~ ch68 use bit 0~9, ch96 ~ ch177 use bit 10~30
 * 				    6G: ch33 ~ ch233 use bit 0~50)
 * 	= 0:	invalid @ch
 */
uint64_t ch2bitmask(enum wl_band_id band, int ch)
{
	if (band < 0 || band >= WL_NR_BANDS)
		return 0;
#if defined(RTCONFIG_WIFI6E)
	else if (is_6g(band))
		return ch6g2bitmask(ch);
#endif
	else if (band == WL_5G_BAND || band == WL_5G_2_BAND)
		return ch5g2bitmask(ch);
	else
		return ch2g2bitmask(ch);
}

/* Convert @bit to channel.
 * @band:	specific wireless band
 * @bit:	bit number. (2G: 0~12 = ch1~13
 * 			     5G: 0~9 = ch32 ~ ch68, 10 ~ 30 = ch96 ~ ch177
 * 			     6G: 0~50 = ch33 ~ ch233)
 * @return:
 * 	>  0:	channel number of @bit
 * 	<= 0:	invalid @bit
 */
int bit2ch(enum wl_band_id band, int bit)
{
	if (band < 0 || band >= WL_NR_BANDS)
		return -1;
#if defined(RTCONFIG_WIFI6E)
	else if (is_6g(band))
		return bit2ch6g(bit);
#endif
	else if (band == WL_5G_BAND || band == WL_5G_2_BAND)
		return bit2ch5g(bit);
	else
		return bit2ch2g(bit);
}

/* Convert channel list @ch_list that is seperate by @sep to bit-mask.
 * @band:	specific wireless band
 * @ch_list:	channel list string, seperated by @sep
 * @sep:	character that is used to seperate channel
 * @return:
 * 	0:	error or none of any legal channel in @ch_list
 *  otherwise:	bit-mask of @ch_list
 */
uint64_t chlist2bitmask(enum wl_band_id band, char *ch_list, char *sep)
{
	if (band < 0 || band >= WL_NR_BANDS)
		return 0;
#if defined(RTCONFIG_WIFI6E)
	else if (is_6g(band))
		return chlist6g2bitmask(ch_list, sep);
#endif
	else if (band == WL_5G_BAND || band == WL_5G_2_BAND)
		return chlist5g2bitmask(ch_list, sep);
	else
		return chlist2g2bitmask(ch_list, sep);
}

/* Convert bit-mask to channel list that is seperated by @sep.
 * @mask:	bit-mask of channels
 * @ch_list:	channel list buffer
 * @ch_list_len: length of @ch_list
 * @return:	pointer to converted string.
 */
char *__bitmask2chlist(enum wl_band_id band, uint64_t mask, char *sep, char *ch_list, size_t ch_list_len)
{
	if (band < 0 || band >= WL_NR_BANDS)
		return "";
#if defined(RTCONFIG_WIFI6E)
	else if (is_6g(band))
		return __bitmask2chlist6g(mask, sep, ch_list, ch_list_len);
#endif
	else if (band == WL_5G_BAND || band == WL_5G_2_BAND)
		return __bitmask2chlist5g(mask, sep, ch_list, ch_list_len);
	else
		return __bitmask2chlist2g(mask, sep, ch_list, ch_list_len);
}

/* Convert bit-mask to channel list that is seperated by @sep.
 * @mask:	bit-mask of channels
 * @return:	pointer to converted string.
 */
char *bitmask2chlist(enum wl_band_id band, uint64_t mask, char *sep)
{
	if (band < 0 || band >= WL_NR_BANDS)
		return "";
#if defined(RTCONFIG_WIFI6E)
	else if (is_6g(band))
		return bitmask2chlist6g(mask, sep);
#endif
	else if (band == WL_5G_BAND || band == WL_5G_2_BAND)
		return bitmask2chlist5g(mask, sep);
	else
		return bitmask2chlist2g(mask, sep);
}

#if defined(RTCONFIG_QCA)
/**
 * Return driver name for wpa_supplicant based on wireless band.
 * @band:	0 ~ MAX_NR_WL_IF
 * @return:	pointer to driver name of @band.
 */
char *get_wsup_drvname(int band)
{
	char *r = "";

	if (absent_band(band))
		return "";

	switch (band) {
	case WL_2G_BAND:	/* fall-through */
	case WL_5G_BAND:	/* fall-through */
	case WL_5G_2_BAND:
		r = (char*) WSUP_DRV;
		break;
	case WL_60G_BAND:
		r = (char*) WSUP_DRV_60G;
		break;
	}

	return r;
}
#endif

/**
 * Check existence of @iface.
 * @return:
 * 	0:	invalid parameter or @iface doesn't exist
 *  otherwise:	@iface exist.
 */
int iface_exist(const char *iface)
{
	char path[sizeof("/sys/class/net/XXXXXX") + IFNAMSIZ];

	if (!iface)
		return 0;

	snprintf(path, sizeof(path), "%s/%s", SYS_CLASS_NET, iface);
	if (!f_exists(path) && !d_exists(path))
		return 0;
	return 1;
}

#ifdef RTCONFIG_TRAFFIC_LIMITER
static char *traffic_limiter_get_path(const char *type)
{
	if (type == NULL)
		return NULL;
	else if (strcmp(type, "limit") == 0)
		return "/jffs/tld/tl_limit";
	else if (strcmp(type, "alert") == 0)
		return "/jffs/tld/tl_alert";
	else if (strcmp(type, "count") == 0)
		return "/jffs/tld/tl_count";

	return NULL;
}

unsigned int traffic_limiter_read_bit(const char *type)
{
	char *path;
	char buf[sizeof("4294967295")];
	unsigned int val = 0;

	path = traffic_limiter_get_path(type);
	if (path && f_read_string(path, buf, sizeof(buf)) > 0)
		val = strtoul(buf, NULL, 10);

	TL_DBG("path = %s, val=%u\n", path ? : "NULL", val);
	return val;
}

void traffic_limiter_set_bit(const char *type, int unit)
{
	char *path;
	char buf[sizeof("4294967295")];
	unsigned int val = 0;

	path = traffic_limiter_get_path(type);
	if (path) {
		val = traffic_limiter_read_bit(type);
		val |= (1U << unit);
		snprintf(buf, sizeof(buf), "%u", val);
		f_write_string(path, buf, 0, 0);
	}

	TL_DBG("path = %s, val=%u\n", path ? : "NULL", val);
}

void traffic_limiter_clear_bit(const char *type, int unit)
{
	char *path;
	char buf[sizeof("4294967295")];
	unsigned int val = 0;

	path = traffic_limiter_get_path(type);
	if (path) {
		val = traffic_limiter_read_bit(type);
		val &= ~(1U << unit);
		snprintf(buf, sizeof(buf), "%u", val);
		f_write_string(path, buf, 0, 0);
	}

	TL_DBG("path = %s, val=%u\n", path ? : "NULL", val);
}

double traffic_limiter_get_realtime(int unit)
{
	char path[PATH_MAX];
	char buf[32];
	double val = 0;

	snprintf(path, sizeof(path), "/tmp/tl%d_realtime", unit);
	if (f_read_string(path, buf, sizeof(buf)) > 0)
		val = atof(buf);

	return val;
}
#endif

#if defined(RTCONFIG_COOVACHILLI)
void deauth_guest_sta(char *wlif_name, char *mac_addr)
{
#if (defined(RTCONFIG_RALINK) || defined(RTCONFIG_QCA))
        char cmd[128];
#if defined(RTCONFIG_RALINK)
		snrintf(cmd, sizeof(cmd), "iwpriv %s set DisConnectSta="MAC_FMT, wlif_name, MAC_ARG((unsigned char *)mac_addr));
		system(cmd);
		memset(cmd, 0, sizeof(cmd));
		snprintf(cmd, sizeof(cmd), "iwpriv %s set AccessPolicy=2", wlif_name);
		system(cmd);
		memset(cmd, 0, sizeof(cmd));
		snprintf(cmd, sizeof(cmd), "iwpriv %s set ACLAddEntry="MAC_FMT, wlif_name,  MAC_ARG((unsigned char *)mac_addr));
		system(cmd);
		memset(cmd, 0, sizeof(cmd));
		snprintf(cmd, sizeof(cmd), "iwpriv %s set ACLShowAll=1", wlif_name);
#elif defined(RTCONFIG_QCA)
	snprintf(cmd, sizeof(cmd), IWPRIV " %s maccmd 2", wlif_name);
        system(cmd);
	memset(cmd, 0, sizeof(cmd));
	snprintf(cmd, sizeof(cmd), IWPRIV " %s addmac "MAC_FMT, wlif_name, MAC_ARG(mac_addr));
        system(cmd);
	memset(cmd, 0, sizeof(cmd));
        snprintf(cmd, sizeof(cmd), IWPRIV " %s kickmac "MAC_FMT, wlif_name, MAC_ARG(mac_addr));
#endif
        system(cmd);
#else /* BCM */
        int ret;
        scb_val_t scb_val;

        memcpy(&scb_val.ea, mac_addr, ETHER_ADDR_LEN);
        scb_val.val = 8; /* reason code: Disassociated because sending STA is leaving BSS */

        ret = wl_ioctl(wlif_name, WLC_SCB_DEAUTHENTICATE_FOR_REASON, &scb_val, sizeof(scb_val));
        if(ret < 0) {
                //printf("[WARNING] error to deauthticate ["MAC_FMT"] !!!\n", MAC_ARG(msc_addr));
        }
#endif
}

int FindBrifByWlif(const char *wl_ifname, char *brif_name, int size)
{
	char word[32]={0}, *next=NULL;

	if( !wl_ifname || !brif_name || !strlen(wl_ifname) )
		return 0;

	if (nvram_match("chilli_enable", "1")){
		foreach(word, nvram_safe_get("chilli_interface"), next){
			if(!strncmp(word, wl_ifname, strlen(word))){
				memset(brif_name, 0, strlen(brif_name));
				strncpy(brif_name, "br1", 3);
			}	
		}
	}
	if (nvram_match("cp_enable", "1")){
		foreach(word, nvram_safe_get("cp_interface"), next){
			if(!strncmp(word, wl_ifname, strlen(word))){ 
				memset(brif_name, 0, strlen(brif_name));
				strncpy(brif_name, "br2", 3);
			}	

		}
	}
	return 0;
}

#endif

#ifdef RTCONFIG_CFGSYNC
int is_valid_group_id(const char *str)
/* valid group id char : "0123456789ABCDEF" */
{
	int i;
	for (i=0; i<CFGSYNC_GROUPID_LEN; i++) {
		if (str[i] >= '0' && str[i] <= '9')
			continue;
		else if (str[i] >= 'A' && str[i] <= 'F')
			continue;
		else
			return 0;
	}
	return 1;
}

char *if_nametoalias(char *name, char *alias, int alias_len)
{
	char prefix[]="wlXXXXXX_", tmp[100];
	char word[256], *next;
	int unit = 0;
	int subunit = 0;
	int max_mssid = 0;
	char wl_ifnames[32] = { 0 };
	char ifname[IFNAMSIZ] = { 0 };
	int found = 0;
	char band_prefix[8];
	char nband = 0, num5g = 0;

	if (!strncmp(name, CFG_WL_STR_2G, 2) || !strncmp(name, CFG_WL_STR_5G, 2) ||
		!strncmp(name, CFG_WL_STR_6G, 2)) {
		snprintf(alias, alias_len, "%s", name);
		return alias;
	}

	strlcpy(wl_ifnames, nvram_safe_get("wl_ifnames"), sizeof(wl_ifnames));
	foreach (word, wl_ifnames, next) {
		SKIP_ABSENT_BAND_AND_INC_UNIT(unit);
		max_mssid = num_of_mssid_support(unit);
		memset(prefix, 0, sizeof(prefix));
		snprintf(prefix, sizeof(prefix), "wl%d_", unit);
		strlcpy(ifname, nvram_safe_get(strlcat_r(prefix, "ifname", tmp, sizeof(tmp))), sizeof(ifname));
		subunit = 0;
		nband = nvram_get_int(strlcat_r(prefix, "nband", tmp, sizeof(tmp)));

		if (nband == 2)
			strlcpy(band_prefix, CFG_WL_STR_2G, sizeof(band_prefix));
		else if (nband == 1)
		{
			num5g++;
#if defined(RTCONFIG_LYRA_5G_SWAP)
			strlcpy(band_prefix, swap_5g_band(unit) == 2 ? CFG_WL_STR_5G1 : CFG_WL_STR_5G, sizeof(band_prefix));
#else
			strlcpy(band_prefix, num5g == 1 ? CFG_WL_STR_5G : CFG_WL_STR_5G1, sizeof(band_prefix));
#endif
		}
		else if (nband == 4)
			strlcpy(band_prefix, CFG_WL_STR_6G, sizeof(band_prefix));

		if (!strcmp(ifname, name)) {
			snprintf(alias, alias_len, "%s", band_prefix);
			found = 1;
			break;
		}

		for (subunit = 1; subunit < max_mssid+1; subunit++) {
			memset(prefix, 0, sizeof(prefix));
                        snprintf(prefix, sizeof(prefix), "wl%d.%d_", unit, subunit);
			strlcpy(ifname, nvram_safe_get(strlcat_r(prefix, "ifname", tmp, sizeof(tmp))), sizeof(ifname));

			if (!strcmp(ifname, name)) {
#if defined(CONFIG_BCMWL5) || defined(RTCONFIG_BCMARM)
				if ((repeater_mode()
					|| dpsr_mode()
#if defined(RTCONFIG_PROXYSTA) && defined(RTCONFIG_DPSTA)
					|| dpsta_mode()
#endif
					|| rp_mode()
					) && subunit == 1)
					snprintf(alias, alias_len, "%s", band_prefix);
				else
#endif
				snprintf(alias, alias_len, "%s_%d", band_prefix, subunit);
				found = 1;
				break;
			}
		}

		if (found)
			break;
 
		unit++;
	}

	return alias;
}

#ifdef CONFIG_BCMWL5
int is_rp_configured() 
{
	if( nvram_match("x_Setting", "1") && rp_mode() &&
	  ((nvram_match("wl0_mode", "wet") && !nvram_match("wl0_ssid", "")) ||
	   (nvram_match("wl1_mode", "wet") && !nvram_match("wl1_ssid", ""))
	  )
	)
		return 1;
	else
		return 0;
}
#endif

int check_re_in_macfilter(int unit, char *mac)
{
	char tmp[128], prefix[] = "wlXXXXXXXXXX_";
	char *nv, *nvp, *b;
	int exist = 0;

#ifdef RTCONFIG_AMAS
	if (nvram_get_int("re_mode") == 1)
		snprintf(prefix, sizeof(prefix), "wl%d.1_", unit);
	else
#endif
		snprintf(prefix, sizeof(prefix), "wl%d_", unit);

	nv = nvp = strdup(nvram_safe_get(strlcat_r(prefix, "maclist_x", tmp, sizeof(tmp))));
	if (nv) {
		while ((b = strsep(&nvp, "<")) != NULL) {
			if (strlen(b) == 0) continue;

			if (strcmp(mac, b) == 0) {
				dbg("mac (%s) exists in maclist_x (%s)\n", mac, prefix);
				exist = 1;
				break;
			}
		}
		free(nv);
	}

	return exist;
}

char *get_cfg_relist(int xfile)
{
	char path[32] = {0}, buf[8192] = {0};

	if (nvram_get_int("re_mode") == 1) {
		snprintf(path, sizeof(path), CFG_RELIST_FILE"%s", xfile ? "_x": "");

		if (f_exists(path))
			f_read_string(path, buf, sizeof(buf));
	}
	else
		strlcpy(buf, xfile ? nvram_safe_get("cfg_relist_x"): nvram_safe_get("cfg_relist"), sizeof(buf));

	return strdup(buf);
}

int is_cfg_relist_exist()
{
	int ret = 0;

	if ((nvram_get_int("re_mode") == 0 && nvram_get("cfg_relist") && strlen(nvram_safe_get("cfg_relist"))) ||
		((nvram_get_int("re_mode") == 1 && f_exists(CFG_RELIST_FILE))))
		ret = 1;

	return ret;
}
#endif /* RTCONFIG_CFGSYNC */

#ifdef RTCONFIG_BONDING
#ifdef HND_ROUTER

#define SYS_BOND_FILE "/proc/net/bonding/bond0"
#define STR_PARTNER_MAC "Partner Mac Address: "

enum {
	BS_NOCONFIG,
	BS_NOPROC,
	BS_NOLINK,
	BS_FAIL,
	BS_OK
};

char *bs_desc[] = {
	"option disabled",
	"no report",
	"not linked",
	"not established",
	"successfully"
};

int get_bonding_status()
{
	FILE *fp;
	char line[128], *lp, *mp=NULL;
	int ret = 0;

	if(!nvram_match("lacp_enabled", "1"))
		return BS_NOCONFIG;

	fp = fopen(SYS_BOND_FILE, "r");

	if(fp != NULL)
	{
		while(fgets(line, sizeof(line), fp) != NULL){
			if((lp = strstr(line, STR_PARTNER_MAC)) != NULL) {
				mp = lp + strlen(STR_PARTNER_MAC);
				mp[17] = 0;
				break;
			}
		}

		fclose(fp);

		if(mp == NULL)
			ret = BS_NOLINK;
		else if(strncmp(mp, "00:00:00:00:00:00", 17) == 0)
			ret = BS_FAIL;
		else
			ret = BS_OK;
	} else {
		ret = BS_NOPROC;
	}

	return ret;
}

#endif
#endif

#ifdef RTCONFIG_LANTIQ
int ppa_support(int wan_unit)
{
	int ret = 1;
	char wan_proto[16];
	char tmp[100], prefix[32];
	int ctf_disable_force = 0 ;

	ctf_disable_force = nvram_get_int("ctf_disable_force");

	/* recover the NAT accelerator*/
	// nvram_set("ctf_disable_force", "0");

#ifdef RTCONFIG_USB_MODEM
	char modem_type[32];
	int wan_type = get_dualwan_by_unit(wan_unit);
	int modem_unit = get_modemunit_by_type(wan_type);
	char tmp2[100], prefix2[32];

	if(modem_unit < 0)
		modem_unit = 0;

	usb_modem_prefix(modem_unit, prefix2, sizeof(prefix2));
	snprintf(modem_type, sizeof(modem_type), "%s", nvram_safe_get(strlcat_r(prefix2, "act_type", tmp2, sizeof(tmp2))));

	if((wan_type == WANS_DUALWAN_IF_USB || wan_type == WANS_DUALWAN_IF_USB2)
			&& (!strcmp(modem_type, "tty") || !strcmp(modem_type, "mbim"))
			)
	{
		ret = 0;
	}
#endif

	/*
		1. traditaional qos / bandwidth limiter / disable NAT accelerator
		2. stop_ppa_wan : debug usage
	*/
	if(IS_NON_AQOS())
	{
		ret = 0;
	}

	if(ctf_disable_force == 1) ret = 0;

	if (nvram_match("stop_ppa_wan", "1")) ret = 0;

	snprintf(prefix, sizeof(prefix), "wan%d_", wan_unit);
	snprintf(wan_proto, sizeof(wan_proto), "%s", nvram_safe_get(strlcat_r(prefix, "proto", tmp, sizeof(tmp))));

	if(strcmp(wan_proto, "pptp") == 0) ret = 0;
	if(strcmp(wan_proto, "l2tp") == 0) ret = 0;
	if(strcmp(nvram_safe_get(strlcat_r(prefix, "hwaddr_x", tmp, sizeof(tmp))), "")) ret = 0;

	return ret;
}

int disable_ppa_wan(char *wan_ifname)
{
	char ppa_cmd[255] = {0};

	if(client_mode() || aimesh_re_mode()){
		system("ppacmd setppefp -f 1");
	}else{
		system("ppacmd setppefp -f 0");
	}
	snprintf(ppa_cmd, sizeof(ppa_cmd), "ppacmd delwan -i %s", wan_ifname);
	_dprintf("[%s][%d] %s\n", __func__, __LINE__, ppa_cmd);
	system(ppa_cmd);
	nvram_set("ctf_disable", "1");
}

int enable_ppa_wan(char *wan_ifname)
{
	char ppa_cmd[255] = {0};
	snprintf(ppa_cmd, sizeof(ppa_cmd), "ppacmd addwan -i %s", wan_ifname);
	_dprintf("[%s][%d] %s\n", __func__, __LINE__, ppa_cmd);
	system(ppa_cmd);
	system("ppacmd setppefp -f 1");
	nvram_set("ctf_disable", "0");
}
#endif

#ifdef RTCONFIG_MULTICAST_IPTV
#define MOVISTART_PORT 1
#else
#define MOVISTART_PORT 3
#endif
int IPTV_ports_cnt(void)
{
	int cnt = 0;
	const char *switch_wantag = nvram_safe_get("switch_wantag");

	if (switch_wantag[0] == '\0' || strcmp(switch_wantag, "none") == 0)
	{
		int switch_stb_x = nvram_get_int("switch_stb_x");
		if (switch_stb_x >= 1 && switch_stb_x <= 4)
			cnt = 2;
		else if (switch_stb_x >= 5 && switch_stb_x <= 6)
			cnt = 3;
	}
	else if (strcmp(switch_wantag, "unifi_biz") == 0)
		cnt = 1;
	else if (strcmp(switch_wantag, "singtel_mio") == 0 
	      || strcmp(switch_wantag, "vodafone") == 0)
		cnt = 3;
	else if (strcmp(switch_wantag, "movistar") == 0) {
		cnt = MOVISTART_PORT;
	}
	else if (strcmp(switch_wantag, "manual") == 0) {
		if (nvram_safe_get("switch_wan0tagid")[0] != 0)
			cnt++;
		if (nvram_safe_get("switch_wan1tagid")[0] != 0)
			cnt++;
		if (nvram_safe_get("switch_wan2tagid")[0] != 0)
			cnt++;
	}
	else
		cnt = 2;
	return cnt;
}

/*
 * Validate a mac address
 * @mac:	pointer to mac address.
 * 	1:	Legal mac address
 *  	0:	illegal mac address
 */
int isValidMacAddress(const char* mac) {
	int i=0, s=0;

	while (*mac) {
		if (isxdigit(*mac))
			i++;
		else if (*mac == ':' || *mac == '-') {
			if (i == 0 || i / 2 - 1 != s)
				break;
			++s;
		}
		else
			s = -1;
		++mac;
	}
	return (i == 12 && (s == 5 || s == 0));
}

/*
 * Validate a mac address 
 * @mac:	pointer to mac address.
 * 	1:	Legal mac address and is not a multicast mac address
 *  	0:	illegal mac address or a multicast mac address
 */

int isValidMacAddr_and_isNotMulticast(const char* mac)
{
	int sec_byte;
	int i = 0, s = 0;

	if (strlen(mac) != 17 || !strcmp("00:00:00:00:00:00", mac))
		return 0;

	while (*mac && i < 12) {
		if (isxdigit(*mac)) {
			if (i == 1) {
				sec_byte= strtol(mac, NULL, 16);
				if ((sec_byte & MULTICAST_BIT)){
					_dprintf("isValidMacAddr 1 sec_byte %x %d %d\n",sec_byte,i,s);
					break;
				}
			}
			i++;
		}
		else if (*mac == ':') {
			if (i == 0 || i/2-1 != s){
				_dprintf("isValidMacAddr 2 %d %d\n",i,s);
				break;
			}
			++s;
		}
		++mac;
	}
	return (i == 12 && s == 5);
}
/*
 * Validate a option input
 * @option:	pointer to  a option.
    @range:	unsigned int range.
 * 	1:	Legal input
 *  	0:	illegal input
 */
int isValidEnableOption(const char *option, int range) {

	int n=0;

	if(!option || strlen(option)!=1)
		return 0;

	n = safe_atoi(option);

	if(n >= 0 && n <=range)
		return 1;
	else
		return 0;
}

/*
 * Validate a string is digit
 * @string:	pointer to  a string.
 */
int isValid_digit_string(const char *string) {

	if (!string || !*string)
		return 0;

	while (*string)
	{
		if (!isdigit(*string))
			return 0;
		else
			++string;
	}
	return 1;
}

/*
 * Validate a char is not valid for a host hame
 * @name:	pointer to  a string.
 */
static int is_invalid_char_for_hostname(int c)
{
	int ret = 0;

	if (c <= 0x2c)				/* space and !"#$%&'()*+, */
		ret = 1;
	else if (c >= 0x2e && c <= 0x2f)	/* ./ */
		ret = 1;
	else if (c >= 0x3a && c <= 0x40)	/* :;<=>?@ */
		ret = 1;
	else if (c >= 0x5b && c <= 0x5e)	/* [\]^ */
		ret = 1;
	else if (c == 0x60)			/* ` */
		ret = 1;
	else if (c >= 0x7b)			/* {|}~ DEL */
		ret = 1;

	return ret;
}

/*
 * Validate a string is valid host name
 * @name:	pointer to  a string.
 */
int is_valid_hostname(const char *name)
{
	const char *p;
	int c;

	if (!name)
		return 0;

	for (p = name; (c = *p); p++) {
		if (is_invalid_char_for_hostname(c))
			return 0;
	}

	return p - name;
}

#if !defined(HND_ROUTER)
void ipt_account(FILE *fp, char *interface) {
	struct in_addr ipaddr, netmask, network;
	char netaddrnetmask[] = "255.255.255.255/255.255.255.255 ";
	int unit;

	inet_aton(nvram_safe_get("lan_ipaddr"), &ipaddr);
	inet_aton(nvram_safe_get("lan_netmask"), &netmask);

	// bitwise AND of ip and netmask gives the network
	network.s_addr = ipaddr.s_addr & netmask.s_addr;

	sprintf(netaddrnetmask, "%s/%s", inet_ntoa(network), nvram_safe_get("lan_netmask"));

	// If we are provided an interface (usually a VPN interface) then use it as WAN.
	if (interface){
		fprintf(fp, "iptables -A ipttolan -i %s -m account --aaddr %s --aname lan -j RETURN\n", interface, netaddrnetmask);
		fprintf(fp, "iptables -A iptfromlan -o %s -m account --aaddr %s --aname lan -j RETURN\n", interface, netaddrnetmask);

	} else {	// Create rules for every WAN interfaces available
		fprintf(fp, "-I FORWARD -i br0 -j iptfromlan\n");
		fprintf(fp, "-I FORWARD -o br0 -j ipttolan\n");
		for (unit = WAN_UNIT_FIRST; unit < WAN_UNIT_MAX; unit++) {
			if ((get_dualwan_by_unit(unit) != WANS_DUALWAN_IF_NONE) && (strlen(get_wan_ifname(unit)))) {
				fprintf(fp, "-A ipttolan -i %s -m account --aaddr %s --aname lan -j RETURN\n", get_wan_ifname(unit), netaddrnetmask);
				fprintf(fp, "-A iptfromlan -o %s -m account --aaddr %s --aname lan -j RETURN\n", get_wan_ifname(unit), netaddrnetmask);
			}
		}
	}
}
#endif

/*
 * Validate a string is valid domain name
 * @name:	pointer to  a string.
 */
int is_valid_domainname(const char *name)
{
	const char *p;
	int c;

	if (!name)
		return 0;

	for (p = name; (c = *p); p++) {
		if (((c | 0x20) < 'a' || (c | 0x20) > 'z') &&
		    ((c < '0' || c > '9')) &&
		    (c != '.' && c != '-' && c != '_'))
			return 0;
	}

	return p - name;
}

int get_discovery_ssid(char *ssid_g, int size)
{
#if defined(RTCONFIG_WIRELESSREPEATER) || defined(RTCONFIG_PROXYSTA)
	char tmp[100], prefix[] = "wlXXXXXXXXXXXXXX";
#endif
#ifdef RTCONFIG_DPSTA
	char word[80], *next;
	int unit, connected;
#endif
#ifdef RTCONFIG_WIRELESSREPEATER
	if (sw_mode() == SW_MODE_REPEATER)
	{
#ifdef RTCONFIG_CONCURRENTREPEATER
		if (nvram_get_int("wlc_band") < 0 || nvram_get_int("wlc_express") == 0)
			snprintf(prefix, sizeof(prefix), "wl0.1_");
		else if (nvram_get_int("wlc_express") == 1)
			snprintf(prefix, sizeof(prefix), "wl1.1_");
		else if (nvram_get_int("wlc_express") == 2)
			snprintf(prefix, sizeof(prefix), "wl0.1_");
		else
#endif
			snprintf(prefix, sizeof(prefix), "wl%d.1_", nvram_get_int("wlc_band"));
		strlcpy(ssid_g, nvram_safe_get(strlcat_r(prefix, "ssid", tmp, sizeof(tmp))), size);
	}
	else
#ifdef RTCONFIG_REALTEK
		if (sw_mode() == SW_MODE_AP && nvram_get_int("wlc_psta") == 1)
		{
#ifdef RTCONFIG_CONCURRENTREPEATER
			snprintf(prefix, sizeof(prefix), "wl0_");
#else
			snprintf(prefix, sizeof(prefix), "wl%d.1_", nvram_get_int("wlc_band"));
#endif
			strlcpy(ssid_g, nvram_safe_get(strlcat_r(prefix, "ssid", tmp, sizeof(tmp))), size);
		}
		else
#endif
#endif
#ifdef RTCONFIG_BCMWL6
#ifdef RTCONFIG_PROXYSTA
#ifdef RTCONFIG_DPSTA
		if (dpsta_mode() && nvram_get_int("re_mode") == 0)
		{
			connected = 0;
			char dpsta_ifnames[32] = { 0 };
			strlcpy(dpsta_ifnames, nvram_safe_get("dpsta_ifnames"), sizeof(dpsta_ifnames));
			foreach(word, dpsta_ifnames, next) {
				wl_ioctl(word, WLC_GET_INSTANCE, &unit, sizeof(unit));
				snprintf(prefix, sizeof(prefix), "wlc%d_", unit == 0 ? 0 : 1);
				if (nvram_get_int(strlcat_r(prefix, "state", tmp, sizeof(tmp))) == 2) {
					connected = 1;
					snprintf(prefix, sizeof(prefix), "wl%d.1_", unit);
					strncpy(ssid_g, nvram_safe_get(strlcat_r(prefix, "ssid", tmp, sizeof(tmp))), size);
					break;
				}
			}
		if (!connected)
			strlcpy(ssid_g, nvram_safe_get("wl0.1_ssid"), size);
		}
		else
#endif
		if (is_psta(nvram_get_int("wlc_band")))
		{
			snprintf(prefix, sizeof(prefix), "wl%d_", nvram_get_int("wlc_band"));
			strncpy(ssid_g, nvram_safe_get(strlcat_r(prefix, "ssid", tmp, sizeof(tmp))), size);
		}
		else if (is_psr(nvram_get_int("wlc_band")))
		{
			snprintf(prefix, sizeof(prefix), "wl%d.1_", nvram_get_int("wlc_band"));
			strlcpy(ssid_g, nvram_safe_get(strlcat_r(prefix, "ssid", tmp, sizeof(tmp))), size);
		}
		else
#endif
#endif
	strlcpy(ssid_g, nvram_safe_get("wl0_ssid"), size);
	return 0;
}

int get_chance_to_control(void)
{
	time_t now_t, login_ts;
	int timeout_threshold;

#ifdef RTCONFIG_FW_JUMP
	timeout_threshold = 300;
#else
	timeout_threshold = 1800;
#endif

	now_t = uptime();
	login_ts = atol(nvram_safe_get("login_timestamp"));
#ifndef RTCONFIG_FW_JUMP
	time_t app_login_ts = atol(nvram_safe_get("app_login_timestamp"));
#endif

	if(((unsigned long)(login_ts) == 0 || (unsigned long)(now_t-login_ts) > timeout_threshold || nvram_match("login_ip", ""))
#ifndef RTCONFIG_FW_JUMP
			&& ((unsigned long)(app_login_ts) == 0 || (unsigned long)(now_t-app_login_ts) > 1800 )
#endif
			) //check httpd from browser not in use
		return 1;
	else
		return 0;
}

int get_index_page(char *page, int size)
{
	if(check_if_file_exist("/www/GameDashboard.asp"))
		strlcpy(page, "GameDashboard.asp", size);
	else
		strlcpy(page, "index.asp", size);
	return 0;
}

/*
	general API to check wss supported interface
*/
int amazon_wss_ap_isolate_support(char *prefix)
{
	char gn_wbl_en[32] = {0};

	/* not amazon_wss interface */
	if (strcmp(prefix, "wl0.2_")) return 0;

	/* amazon_ffs is enabled */
	snprintf(gn_wbl_en, sizeof(gn_wbl_en), "%sgn_wbl_enable", prefix);
	if (nvram_match(gn_wbl_en, "1")) return 1;

	/* mismatch */
	return 0;
}

#ifdef RTCONFIG_AMAS_WGN
int get_iptv_and_dualwan_info(int *iptv_vids,int size, unsigned int *wan_deny_list, unsigned int *lan_deny_list)
{
	char *wans_dualwan = NULL,*switch_wantag=NULL;
	int wans_lanport=0,switch_stb_x=0;
	unsigned int lan_deny_list_tmp=0;

	lan_deny_list_tmp = *lan_deny_list;

	wans_dualwan = nvram_safe_get("wans_dualwan");
	if( wans_dualwan && strstr(wans_dualwan,"lan") )
	{
		wans_lanport = nvram_get_int("wans_lanport");
		if(wans_lanport >=5 && wans_lanport <= 8){
			lan_deny_list_tmp = lan_deny_list_tmp & ~(1 << (wans_lanport-1) );
			lan_deny_list_tmp = lan_deny_list_tmp & ~(1 << (wans_lanport-1 + 16) );
		}
	}

	switch_wantag = strdup(nvram_safe_get("switch_wantag"));

	if(!switch_wantag) {
		*lan_deny_list = lan_deny_list_tmp;
		return 0;
	}

	if( !strcmp(switch_wantag,"none" ) || !strcmp(switch_wantag,"" ) || !strcmp(switch_wantag,"hinet" ) )   
	{
		switch_stb_x = nvram_get_int("switch_stb_x");

		if(switch_stb_x == 1) {
			lan_deny_list_tmp = lan_deny_list_tmp & ~(1 << (1-1));
			lan_deny_list_tmp = lan_deny_list_tmp & ~(1 << (1-1+16));
		}
		else if(switch_stb_x == 2) {
			lan_deny_list_tmp = lan_deny_list_tmp & ~(1 << (2-1));
			lan_deny_list_tmp = lan_deny_list_tmp & ~(1 << (2-1+16));
		}
		else if(switch_stb_x == 3) {
			lan_deny_list_tmp = lan_deny_list_tmp & ~(1 << (3-1));
			lan_deny_list_tmp = lan_deny_list_tmp & ~(1 << (3-1+16));
		}
		else if(switch_stb_x == 4) {
			lan_deny_list_tmp = lan_deny_list_tmp & ~(1 << (4-1));
			lan_deny_list_tmp = lan_deny_list_tmp & ~(1 << (4-1+16));
		}
		else if(switch_stb_x == 5) {
			lan_deny_list_tmp = lan_deny_list_tmp & ~(1 << (1-1));
			lan_deny_list_tmp = lan_deny_list_tmp & ~(1 << (1-1+16));
			lan_deny_list_tmp = lan_deny_list_tmp & ~(1 << (2-1));
			lan_deny_list_tmp = lan_deny_list_tmp & ~(1 << (2-1+16));			
		}
		else if(switch_stb_x == 6 || switch_stb_x == 8) {
			lan_deny_list_tmp = lan_deny_list_tmp & ~(1 << (3-1));
			lan_deny_list_tmp = lan_deny_list_tmp & ~(1 << (3-1+16));
			lan_deny_list_tmp = lan_deny_list_tmp & ~(1 << (4-1));
			lan_deny_list_tmp = lan_deny_list_tmp & ~(1 << (4-1+16));
		}
	} else if ( !strcmp(switch_wantag,"unifi_home" ) ) {
		iptv_vids[0] = 500;
		iptv_vids[1] = 600;
		lan_deny_list_tmp = lan_deny_list_tmp & ~(1 << (4-1));
		lan_deny_list_tmp = lan_deny_list_tmp & ~(1 << (4-1+16));
	} else if ( !strcmp(switch_wantag,"unifi_biz" ) ) {
		iptv_vids[0] = 500;
	} else if ( !strcmp(switch_wantag,"singtel_mio" ) ) {
		iptv_vids[0] = 10;
		iptv_vids[1] = 20;
		iptv_vids[2] = 30;
		lan_deny_list_tmp = lan_deny_list_tmp & ~(1 << (3-1));
		lan_deny_list_tmp = lan_deny_list_tmp & ~(1 << (4-1));
		lan_deny_list_tmp = lan_deny_list_tmp & ~(1 << (3-1+16));
		lan_deny_list_tmp = lan_deny_list_tmp & ~(1 << (4-1+16));
	} else if ( !strcmp(switch_wantag,"singtel_others" ) ) {
		iptv_vids[0] = 10;
		iptv_vids[1] = 20;
		lan_deny_list_tmp = lan_deny_list_tmp & ~(1 << (4-1));//0x00010011
		lan_deny_list_tmp = lan_deny_list_tmp & ~(1 << (4-1+16));		
	} else if ( !strcmp(switch_wantag,"m1_fiber" ) ) {
		iptv_vids[0] = 1103;
		iptv_vids[1] = 1107;
		lan_deny_list_tmp = lan_deny_list_tmp & ~(1 << (3-1));
		lan_deny_list_tmp = lan_deny_list_tmp & ~(1 << (3-1+16));
	} else if ( !strcmp(switch_wantag,"maxis_fiber_sp" ) ) {
		iptv_vids[0] = 11;
		iptv_vids[1] = 14;
		lan_deny_list_tmp = lan_deny_list_tmp & ~(1 << (3-1));
		lan_deny_list_tmp = lan_deny_list_tmp & ~(1 << (3-1+16));
	} else if ( !strcmp(switch_wantag,"maxis_fiber" ) ) {
		iptv_vids[0] = 621;
		iptv_vids[1] = 821;
		iptv_vids[2] = 822;
		lan_deny_list_tmp = lan_deny_list_tmp & ~(1 << (3-1));
		lan_deny_list_tmp = lan_deny_list_tmp & ~(1 << (3-1+16));
	} else if ( !strcmp(switch_wantag,"movistar" ) ) {
		iptv_vids[0] = 2;
		iptv_vids[1] = 3;
		iptv_vids[2] = 6;
		lan_deny_list_tmp = lan_deny_list_tmp & ~(1 << (3-1));
		lan_deny_list_tmp = lan_deny_list_tmp & ~(1 << (4-1));
		lan_deny_list_tmp = lan_deny_list_tmp & ~(1 << (3-1+16));
		lan_deny_list_tmp = lan_deny_list_tmp & ~(1 << (4-1+16));
	} else if ( !strcmp(switch_wantag,"meo" ) ) {
		iptv_vids[0] = 12;
		lan_deny_list_tmp = lan_deny_list_tmp & ~(1 << (4-1));
		lan_deny_list_tmp = lan_deny_list_tmp & ~(1 << (4-1+16));
	} else if ( !strcmp(switch_wantag,"manual" ) ) {
		iptv_vids[0] = nvram_get_int("switch_wan0tagid");
		iptv_vids[1] = nvram_get_int("switch_wan1tagid");;
		iptv_vids[2] = nvram_get_int("switch_wan2tagid");;
		lan_deny_list_tmp = lan_deny_list_tmp & ~(1 << (3-1));
		lan_deny_list_tmp = lan_deny_list_tmp & ~(1 << (4-1));
		lan_deny_list_tmp = lan_deny_list_tmp & ~(1 << (3-1+16));
		lan_deny_list_tmp = lan_deny_list_tmp & ~(1 << (4-1+16));		
	} else if ( !strcmp(switch_wantag,"vodafone" ) ) {
		iptv_vids[0] = 100;
		iptv_vids[1] = 105;
		lan_deny_list_tmp = lan_deny_list_tmp & ~(1 << (3-1));
		lan_deny_list_tmp = lan_deny_list_tmp & ~(1 << (4-1));
		lan_deny_list_tmp = lan_deny_list_tmp & ~(1 << (3-1+16));
		lan_deny_list_tmp = lan_deny_list_tmp & ~(1 << (4-1+16));
	}
	
	*lan_deny_list = lan_deny_list_tmp;
	//printf("lan_deny_list_tmp %x wans_lanport %x\n",lan_deny_list_tmp,wans_lanport);
	free(switch_wantag);
	return 0;
}
#endif	// RTCONFIG_AMAS_WGN

void firmware_downgrade_check(uint32_t sf)
{
	dbg("sf=%04X\n", sf);
#ifdef RTCONFIG_MSSID_PRELINK
	check_mssid_prelink_reset(sf);
#endif
}

/*
 * Returns a valid ddns host name or empty string
 */
char *get_ddns_hostname(void)
{
	char *host = nvram_safe_get("ddns_hostname_x");
	char *server = nvram_safe_get("ddns_server_x");

	if ((strcmp(server, "WWW.DNSOMATIC.COM") == 0 && strcasecmp(host, "all.dnsomatic.com") == 0) ||
	    (strcmp(server, "WWW.TUNNELBROKER.NET") == 0) ||
	    !is_valid_domainname(host)) {
		return "";
	}

	return host;
}

char *get_unused_brif(unsigned int num, char *ret_buffer, size_t ret_buffer_size)
{
	unsigned int i = 0;
	unsigned int total = 0;
	char *ptr = NULL;
	char *end_ptr = NULL;
	char br_ifname[64];
	size_t copy_size = 0;
	
	if (num <= 0 || !ret_buffer || ret_buffer_size <= 0)
		return NULL;

	memset(ret_buffer, 0, ret_buffer_size);
	ptr = &ret_buffer[0];
	end_ptr = ptr + ret_buffer_size;
	for (i=1, total=0, copy_size=0; i<WLIFU_MAX_NO_BRIDGE && total<num && copy_size<ret_buffer_size; i++) 
	{
		memset(br_ifname, 0, sizeof(br_ifname));
		snprintf(br_ifname, sizeof(br_ifname), "br%d", i);
		if (!iface_exist(br_ifname))
		{
			ptr += snprintf(ptr, end_ptr - ptr, "%s ", br_ifname);
			copy_size += strlen(br_ifname) + 1;
			total ++;
		}
	}

	if (copy_size < ret_buffer_size) 
	{
		if (strlen(ret_buffer) > 0)
		{
			ret_buffer[strlen(ret_buffer)-1] = '\0';
		}
	}
	else 
	{
		memset(ret_buffer, 0, ret_buffer_size);	
	}
	
	return (strlen(ret_buffer) > 0) ? ret_buffer : NULL;
}

int is_passwd_default(){
	char *http_passwd = nvram_safe_get("http_passwd");
#ifdef RTCONFIG_NVRAM_ENCRYPT
	char dec_passwd[NVRAM_ENC_LEN];
	pw_dec(http_passwd, dec_passwd, sizeof(dec_passwd));
	http_passwd = dec_passwd;
#endif
	if(strcmp(nvram_default_get("http_passwd"), http_passwd) == 0)
		return 1;
	else
	return 0;
}

int find_clientlist_groupid(char *groupid_list, char *groupname, char *groupid, size_t groupid_len) {

	int have_data = 0;
	char *buf, *g, *p;
	char *groupname_t, *groupid_t;

	g = buf = strdup(groupid_list);

	if(strcmp(buf, "") != 0) {
		while (buf) {
			if ((p = strsep(&g, "<")) == NULL) break;

			if((vstrsep(p, ">", &groupname_t, &groupid_t)) != 2)
				continue;

			if(!strcmp(groupname_t, groupname)){
				strlcpy(groupid, groupid_t, groupid_len);
				have_data = 1;
				break;
			}
		}
	}
	free(buf);
	return have_data;
}

int gen_random_num(int max)
{
	int i, ret;

	srand(time(NULL));
	ret = rand() % max;

        printf("%d\n", ret);

	return ret;
}

void gen_random_string(char *out, size_t len)
{
	int i = 0;
	FILE *fp = NULL;
	char *rand_buf = NULL, *rand_str = NULL;

	rand_buf = malloc(sizeof(char) * (len/2 + 1));
	rand_str = malloc(sizeof(char) * (len + 2));

	if ((fp = fopen("/dev/urandom", "r")) != NULL) {
		fread(rand_buf, 1, len/2 + 1, fp);
		fclose(fp);
	}

	for(i=0;i<len/2+1;i++)
		snprintf(&rand_str[i*2], len + 2, "%02x", rand_buf[i]);

	strlcpy(out, rand_str, len);
	free(rand_buf);
	free(rand_str);
}

void gen_custom_clientlist_groupid(void) {
	int data_count = 0, isfirst = 1, tmp_len = 0;
	char custom_clientlist[CKN_STR_MAX]={0}, init_str[1] = {0}, client_buf[256] = {0}, gen_groupid[11] = {0};
	char *buf, *g, *p, *gvalue;
	char *name, *mac, *group, *type, *callback, *keeparp, *groupname, *age, *groupid;
	g = buf = strdup(nvram_safe_get("custom_clientlist"));
	tmp_len = strlen(buf) + 1;
	gvalue = malloc(tmp_len);

	if(strcmp(buf, "") != 0) {
		while (buf) {
			if ((p = strsep(&g, "<")) == NULL) break;

			data_count = vstrsep(p, ">", &name, &mac, &group, &type, &callback, &keeparp, &groupname, &age, &groupid);
			switch(data_count) {
				case 6:
					groupname = init_str;
				case 7:
					age = init_str;
				case 8:
					groupid = init_str;
				case 9:
					break;
				default:
					continue;
			}

			if(*groupname != '\0' && *groupid == '\0'){
				memset(client_buf, 0, sizeof(client_buf));
				memset(gen_groupid, 0, sizeof(gen_groupid));
				if(find_clientlist_groupid(gvalue, groupname, gen_groupid, sizeof(gen_groupid)) == 0)
					gen_random_string(gen_groupid, sizeof(gen_groupid));

				groupid = gen_groupid;
				snprintf(client_buf, sizeof(client_buf), "<%s>%s", groupname, groupid);
				strlcat(gvalue, client_buf, tmp_len);
			}

			memset(client_buf, 0, sizeof(client_buf));
			if(isfirst == 0)
				strlcat(custom_clientlist, "<", sizeof(custom_clientlist));
			else
				isfirst = 0;

			snprintf(client_buf, sizeof(client_buf), "%s>%s>%s>%s>%s>%s>%s>%s>%s", name, mac, group, type, callback, keeparp, groupname, age, groupid);
			strlcat(custom_clientlist, client_buf, sizeof(custom_clientlist));
		}
	}

	nvram_set("custom_clientlist", custom_clientlist);

	free(buf);
	free(gvalue);
}

#if defined(RTCONFIG_AMAS) && defined(RTCONFIG_PSR_GUEST)
void update_wlx_psr_mbss(void)
{
	int unit = 0, subunit = 0, wlx_psr_mbss = 0;
	char nv[64];

	if (nvram_get_int("re_mode") == 0)
		return;

	for (unit=0; unit<num_of_wl_if(); unit++) {
		wlx_psr_mbss = 0;
		for (subunit=2; subunit<num_of_mssid_support(unit); subunit++) {
			memset(nv, 0, sizeof(nv));
			snprintf(nv, sizeof(nv), "wl%d.%d_bss_enabled", unit, subunit);
			if (nvram_get_int(nv) == 1) {
				wlx_psr_mbss = 1;
				break;
			}
		}

		memset(nv, 0, sizeof(nv));
		snprintf(nv, sizeof(nv), "wl%d_psr_mbss", unit);
		if (nvram_get_int(nv) != wlx_psr_mbss) {
			nvram_set_int(nv, wlx_psr_mbss);
			nvram_commit();
		}
	}

	return;
}
#endif	// #if defined(RTCONFIG_AMAS) && defined(RTCONFIG_PSR_GUEST)

static char *rand_string(char *str, size_t size)
{
	const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJK...";
	size_t n;
	if (size) {
		--size;
		for (n = 0; n < size; n++) {
			int key = rand() % (int) (sizeof charset - 1);
			str[n] = charset[key];
		}
		str[size] = '\0';
	}
	return str;
}

char* rand_string_alloc(size_t size)
{
	char *s = malloc(size + 1);
	if (s) {
		rand_string(s, size);
	}
	return s;
}

void set_rannv(const char *nvname, size_t n)
{
	char *val = rand_string_alloc(n);

	if(val) {
		nvram_set(nvname, val);
		free(val);
	} else
		_dprintf("%s:%s set fail\n", __func__, nvname);	

}

int get_ispctrl()
{
#if defined(RTCONFIG_ELINK)
	if (nvram_get_int("elink_enable") && nvram_get_int("amas_disable"))
		return 1;
#endif
#if defined(RTCONFIG_XLN_WIFI)
	return 1;
#endif
	return 0;
}

unsigned short get_extend_cap()
{
	unsigned short extend_cap = 0;

#ifdef RTCONFIG_WEBDAV
	extend_cap |= __cpu_to_le16(EXTEND_CAP_WEBDAV);
#else
	extend_cap = 0;
	if(check_if_file_exist("/opt/etc/init.d/S50aicloud"))
		extend_cap |= __cpu_to_le16(EXTEND_CAP_WEBDAV);
#endif

#ifdef RTCONFIG_TUNNEL
	extend_cap |= __cpu_to_le16(EXTEND_CAP_AAE_BASIC);
#endif

	extend_cap |= __cpu_to_le16(EXTEND_CAP_SWCTRL);

#ifdef RTCONFIG_AMAS
#ifdef RTCONFIG_SW_HW_AUTH
	if (getAmasSupportMode() != 0)
	{
#endif
		if (!repeater_mode()
#if defined(RTCONFIG_BCMWL6) && defined(RTCONFIG_PROXYSTA)
			&& !psr_mode()
#endif
#ifdef RTCONFIG_DPSTA
			&& !(dpsta_mode() && nvram_get_int("re_mode") == 0)
#endif
		)
			extend_cap |= __cpu_to_le16(EXTEND_CAP_AMAS);
#ifdef RTCONFIG_SW_HW_AUTH
	}
#endif
	if (nvram_get_int("amas_bdl"))
		extend_cap |= __cpu_to_le16(EXTEND_CAP_AMAS_BDL);
#endif
#if defined(RTCONFIG_CFGSYNC) && defined(RTCONFIG_MASTER_DET)
	if (nvram_get_int("cfg_master"))
		extend_cap |= __cpu_to_le16(EXTEND_CAP_MASTER);
#endif

	if (get_ispctrl())
		extend_cap |= __cpu_to_le16(EXTEND_CAP_ISPCTRL_LOGIN);

       return extend_cap;
}
