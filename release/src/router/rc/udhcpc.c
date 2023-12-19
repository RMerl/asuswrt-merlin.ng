/*
 * udhcpc scripts
 *
 * Copyright (C) 2009, Broadcom Corporation. All Rights Reserved.
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * $Id: udhcpc.c,v 1.27 2009/12/02 20:06:40 Exp $
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <net/ethernet.h>
#include <asm/byteorder.h>

#include <bcmnvram.h>
#include <shutils.h>
#include <rc.h>

#ifdef RTCONFIG_AMAS
#include <amas-utils.h>
#endif

/* Support for Domain Search List */
#undef DHCP_RFC3397

#ifdef RTCONFIG_SOFTWIRE46
extern char **environ;
#endif
/* returns: length of hex value
 * dest size must be lagre enough to accept n bytes from
   src in hex representation plus one \0 byte */
static int
bin2hex(char *dest, size_t size, const void *src, size_t n)
{
	unsigned char *sptr = (unsigned char *) src;
	unsigned char *send = sptr + n;
	char *dptr = dest;

	while (sptr < send && size > 2) {
		n = snprintf(dptr, size, "%02x", *sptr++);
		dptr += n;
		size -= n;
	}
	return dptr - dest;
}

#if defined(RTCONFIG_TR069) || (defined(RTCONFIG_AMAS) && defined(RTCONFIG_PRELINK))
/* returns:
 *  NULL on NULL value or alloc error
 *  binary representation of hex value on success
 * if size if specified, returns actual size of parsed value
 * note: value is alloced and needs to be free() */
static char *
hex2bin(const char *value, size_t *size)
{
	char buf[3], *src, *tmp;
	unsigned char *dst, *ptr;

	if (!(src = (char *) value) ||
	    !(dst = calloc(1, strlen(src)/2 + 1)))
		return NULL;

	memset(buf, 0, sizeof(buf));
	for (ptr = dst; src[0] && src[1]; ptr++) {
		buf[0] = *src++;
		buf[1] = *src++;
		*ptr = strtoul(buf, &tmp, 16);
		if (tmp == buf || *tmp)
			break;
	}
	if (size)
		*size = ptr - dst;

	return (char *) dst;
}
#endif

/* safely check nvram value
 * returns:
 *  0 if not equal
 *  1 if equal */
static int
nvram_check(const char *name, const char *value)
{
	char *nvalue = nvram_get(name);

	return (nvalue == value || strcmp(nvalue ? : "", value ? : "") == 0);
}

/* set nvram to value
 * returns:
 *  0 if not changed
 *  1 if new/changed/removed */
static int
nvram_set_check(const char *name, const char *value)
{
	if (nvram_check(name, value))
		return 0;

	nvram_set(name, value);
	return 1;
}

/* set nvram to env value
 * returns:
 *  0 if not changed
 *  1 if new/changed/removed */
static int
nvram_set_env(const char *name, const char *env)
{
	char *evalue = getenv(env);

	if (evalue)
		evalue = trim_r(evalue);

	return nvram_set_check(name, evalue ? : "");
}

struct viopt_hdr {
	unsigned int entnum;
	unsigned char len;
	unsigned char data[0];
} __attribute__ ((__packed__));

struct opt_hdr {
	unsigned char id;
	unsigned char len;
	unsigned char data[0];
} __attribute__ ((__packed__));

#if defined(RTCONFIG_TR069) || (defined(RTCONFIG_AMAS) && defined(RTCONFIG_PRELINK))
#if defined(RTCONFIG_TR181) || (defined(RTCONFIG_AMAS) && defined(RTCONFIG_PRELINK))
static struct viopt_hdr *
viopt_get(const void *buf, size_t size, unsigned int entnum)
{
	struct viopt_hdr *opt;
	unsigned char *ptr = (unsigned char *) buf;
	unsigned char *end = (unsigned char *) buf + size;

	while (ptr + sizeof(*opt) <= end) {
		opt = (struct viopt_hdr *) ptr;
		if ((ptr += sizeof(*opt) + opt->len) > end)
			break;
		if (opt->entnum == entnum)
			return opt;
	}

	return NULL;
}
#endif

static struct opt_hdr *
opt_get(const void *buf, size_t size, unsigned char id)
{
	struct opt_hdr *opt;
	unsigned char *ptr = (unsigned char *) buf;
	unsigned char *end = (unsigned char *) buf + size;

	while (ptr + sizeof(*opt) <= end) {
		opt = (struct opt_hdr *) ptr;
		if (opt->id == 0) {
			ptr++;
			continue;
		} else if (opt->id == 255)
			break;
		if ((ptr += sizeof(*opt) + opt->len) > end)
			break;
		if (opt->id == id)
			return opt;
	}

	return NULL;
}

#ifdef RTCONFIG_TR069
static char
*stropt(const struct opt_hdr *opt, char *buf)
{
	*stpncpy(buf, (char *)opt->data, opt->len) = '\0';
	return buf;
}

#if defined(RTCONFIG_TR181) || defined(RTCONFIG_DSL)
static int
opt_add(const void *buf, size_t size, unsigned char id, void *data, unsigned char len)
{
	struct opt_hdr *opt = (struct opt_hdr *) buf;

	if (size >= sizeof(*opt) + len) {
		opt->id = id;
		opt->len = len;
		memcpy(opt->data, data, len);
		return sizeof(*opt) + len;
	}

	return 0;
}
#endif
#endif
#endif

struct duid {
	uint16_t type;
	uint16_t hwtype;
	unsigned char ea[ETHER_ADDR_LEN];
} __attribute__ ((__packed__));

/* Generate DUID-LL */
static int get_duid(struct duid *duid)
{
	/* Use device default MAC */
	if (!duid || !ether_atoe(get_label_mac(), duid->ea))
		return 0;

	duid->type = htons(3);		/* DUID-LL */
	duid->hwtype = htons(1);	/* Ethernet */

	return ETHER_ADDR_LEN;
}

static int
expires(char *wan_ifname, unsigned int in)
{
	char tmp[100], prefix[sizeof("wanXXXXXXXXXX_")];
	int unit;
	time_t now;
	FILE *fp;

	if ((unit = wan_prefix(wan_ifname, prefix)) < 0)
		return -1;
	if (wan_ifunit(wan_ifname) < 0)
		snprintf(prefix, sizeof(prefix), "wan%d_x", unit);

	nvram_set_int(strcat_r(prefix, "expires", tmp), (unsigned int) uptime() + in);

	snprintf(tmp, sizeof(tmp), "/tmp/udhcpc%d.expires", unit);
	if ((fp = fopen(tmp, "w")) == NULL) {
		perror(tmp);
		return errno;
	}
	time(&now);
	fprintf(fp, "%d", (unsigned int) now + in);
	fclose(fp);

	return 0;
}

/* 
 * deconfig: This argument is used when udhcpc starts, and when a
 * leases is lost. The script should put the interface in an up, but
 * deconfigured state.
*/
static int
deconfig(int zcip)
{
	char *wan_ifname = safe_getenv("interface");
	char tmp[100], prefix[sizeof("wanXXXXXXXXXX_")];
	int unit = wan_ifunit(wan_ifname);
	int end_wan_sbstate = WAN_STOPPED_REASON_DHCP_DECONFIG;

	/* Figure out nvram variable name prefix for this i/f */
	if (wan_prefix(wan_ifname, prefix) < 0)
		return -1;
	if ((unit < 0) &&
	    (nvram_match(strcat_r(prefix, "proto", tmp), "l2tp") ||
	     nvram_match(strcat_r(prefix, "proto", tmp), "pptp"))) {
		logmessage(zcip ? "zcip client" : "dhcp client", "skipping resetting IP address to 0.0.0.0");
	} else
		ifconfig(wan_ifname, IFUP, "0.0.0.0", NULL);

	expires(wan_ifname, 0);
	wan_down(wan_ifname);

#if defined(RTCONFIG_JFFS2) || defined(RTCONFIG_BRCM_NAND_JFFS2) || defined(RTCONFIG_UBIFS)
	if (get_wan_sbstate(unit) == WAN_STOPPED_REASON_DATALIMIT)
		end_wan_sbstate = WAN_STOPPED_REASON_DATALIMIT;
#endif

	/* Skip physical VPN subinterface */
	if (!(unit < 0))
		update_wan_state(prefix, WAN_STATE_STOPPED, end_wan_sbstate);

	_dprintf("udhcpc:: %s done\n", __FUNCTION__);
	return 0;
}

/*
 * bound: This argument is used when udhcpc moves from an unbound, to
 * a bound state. All of the paramaters are set in enviromental
 * variables, The script should configure the interface, and set any
 * other relavent parameters (default gateway, dns server, etc).
*/
static int
bound(int renew)
{
	char *wan_ifname = safe_getenv("interface");
	char *value;
	char tmp[100], prefix[sizeof("wanXXXXXXXXXX_")];
	char wanprefix[sizeof("wanXXXXXXXXXX_")];
	int unit, ifunit;
	int changed = 0;
#if defined(RTCONFIG_PORT_BASED_VLAN) || defined(RTCONFIG_TAGGED_BASED_VLAN)
	char ip_mask[sizeof("192.168.100.200/255.255.255.255XXX")];
#endif
#if defined(RTCONFIG_TR069) || (defined(RTCONFIG_AMAS) && defined(RTCONFIG_PRELINK))
	size_t size = 0;
#endif
#if defined(RTCONFIG_USB_MODEM) && defined(RTCONFIG_INTERNAL_GOBI)
	int modem_unit;
	char tmp2[100], prefix2[32];
#endif
#ifdef RTCONFIG_AUTO_WANPORT
	char lan_ifname[16], lan_ip[16], lan_net[16];
	char autowan_detected_ifname[8];
	char gateway[16], gateway_mac[32];
	int br_no;
	char if_name[16];

	if(is_auto_wanport_enabled() > 0){
		strlcpy(autowan_detected_ifname, nvram_safe_get("autowan_detected_ifname"), sizeof(autowan_detected_ifname));

		if(strcmp(autowan_detected_ifname, "") && strcmp(autowan_detected_ifname, wan_ifname)){
			_dprintf("%s(%lu): auto_wanport: Had detected the WAN interface. skip bounding of %s.\n", __func__, getpid(), wan_ifname);
			return -1;
		}

		// start_wan() will run faster than wanduck, so getting DHCP IP is in front of the PPPoE detect possiably
		// Need to do the PPPoE detect once if the WAN port is not set
		if(!strcmp(autowan_detected_ifname, "")){
			strlcpy(gateway, getenv("router"), sizeof(gateway));
			strlcpy(lan_ifname, nvram_safe_get("lan_ifname"), sizeof(lan_ifname));
			strlcpy(lan_ip, nvram_safe_get("lan_ipaddr"), sizeof(lan_ip));
			strlcpy(lan_net, nvram_safe_get("lan_netmask"), sizeof(lan_net));

			_dprintf("%s(%lu): auto_wanport: Get gateway %s & apply to %s first.\n", __func__, getpid(), gateway, lan_ifname);
			ifconfig(lan_ifname, IFUP, "0.0.0.0", NULL);
			ifconfig(lan_ifname, IFUP, getenv("ip"), getenv("subnet"));

			_dprintf("%s(%lu): auto_wanport: send a ping to gateway for ARP.\n", __func__, getpid());
			eval("ping", "-W1", "-c1", gateway);

			memset(gateway_mac, 0, sizeof(gateway_mac));
			get_mac_from_ip(gateway, gateway_mac, sizeof(gateway_mac));
			if(!gateway_mac[0]){
				_dprintf("%s(%lu): auto_wanport: Fail to get gateway_mac & restore the LAN IP of router.\n", __func__, getpid());
				ifconfig(lan_ifname, IFUP, "0.0.0.0", NULL);
				ifconfig(lan_ifname, IFUP, lan_ip, lan_net);
				return -1;
			}
			_dprintf("%s(%lu): auto_wanport: Got gateway's MAC %s.\n", __func__, getpid(), gateway_mac);

			br_no = get_br_port_no_from_mac(gateway_mac);
			if(br_no < 0){
				_dprintf("%s(%lu): auto_wanport: Canot get gateway's br_no.\n", __func__, getpid());
				ifconfig(lan_ifname, IFUP, "0.0.0.0", NULL);
				ifconfig(lan_ifname, IFUP, lan_ip, lan_net);
				return -1;
			}

			_dprintf("%s(%lu): auto_wanport: Got gateway's br_no %d.\n", __func__, getpid(), br_no);
			get_if_from_br_port_no(br_no, if_name, sizeof(if_name));
			_dprintf("%s(%lu): auto_wanport: Got gateway's if %s.\n", __func__, getpid(), if_name);
			wan_ifname = if_name;

			_dprintf("%s(%lu): auto_wanport: restore the LAN IP of router.\n", __func__, getpid());
			ifconfig(lan_ifname, IFUP, "0.0.0.0", NULL);
			ifconfig(lan_ifname, IFUP, lan_ip, lan_net);

			_dprintf("%s(%lu): auto_wanport: Choose the WAN interface %s because of DHCP.\n", __func__, getpid(), wan_ifname);
			set_auto_wanport(wan_ifname, 1);

			return -1;
		}

		ifunit = nvram_get_int("autowan_live_wanunit");
		snprintf(wanprefix, sizeof(wanprefix), "wan%d_", ifunit);
	}
	else
#endif
	/* Figure out nvram variable name prefix for this i/f */
	if ((ifunit = wan_prefix(wan_ifname, wanprefix)) < 0)
		return -1;

	if ((unit = wan_ifunit(wan_ifname)) < 0
#ifdef RTCONFIG_SOFTWIRE46
	    || nvram_pf_get_int(wanprefix, "s46_hgw_case") == S46_CASE_MAP_HGW_OFF
#endif
	)
		snprintf(prefix, sizeof(prefix), "wan%d_x", ifunit);
	else
		snprintf(prefix, sizeof(prefix), "wan%d_", ifunit);

	snprintf(tmp, sizeof(tmp), "/tmp/%sbound.env", prefix);
	envsave(tmp);

	/* Stop zcip to avoid races */
	stop_zcip(ifunit);

	changed += nvram_set_env(strcat_r(prefix, "ipaddr", tmp), "ip");
#if defined(RTCONFIG_USB_MODEM) && defined(RTCONFIG_INTERNAL_GOBI)
	if (dualwan_unit__usbif(ifunit)) {
		modem_unit = get_modemunit_by_dev(wan_ifname);
		if (modem_unit == MODEM_UNIT_NONE) {
			_dprintf("%s: cannot get the modem unit!\n", __FUNCTION__);
			return -1;
		}

		usb_modem_prefix(modem_unit, prefix2, sizeof(prefix2));
	}

	if (dualwan_unit__usbif(ifunit) && nvram_match(strcat_r(prefix2, "act_type", tmp2), "gobi")) {
		changed += nvram_set_check(strcat_r(prefix, "netmask", tmp), "255.255.255.255");
		changed += nvram_set_env(strcat_r(prefix, "gateway", tmp), "ip");
	} else
#endif
	{
		changed += nvram_set_env(strcat_r(prefix, "netmask", tmp), "subnet");
		changed += nvram_set_env(strcat_r(prefix, "gateway", tmp), "router");
	}

	// always backup received dns
	if ((value = getenv("dns") ? : getenv("router")))
		nvram_set(strlcat_r(prefix, "dns_r", tmp, sizeof(tmp)), trim_r(value));

	if (nvram_get_int(strcat_r(wanprefix, "dnsenable_x", tmp))) {
		/* ex: android phone, the gateway is the DNS server. */
		if ((value = getenv("dns") ? : getenv("router")))
			nvram_set(strcat_r(prefix, "dns", tmp), trim_r(value));
#ifdef DHCP_RFC3397
		if ((value = getenv("search")) && *value) {
			char *domain, *result;
			if ((domain = getenv("domain")) && *domain &&
			    find_word(value, trim_r(domain)) == NULL) {
				result = alloca(strlen(domain) + strlen(value) + 2);
				sprintf(result, "%s %s", domain, value);
				value = result;
			}
			nvram_set(strcat_r(prefix, "domain", tmp), trim_r(value));
		} else
#endif
		nvram_set_env(strcat_r(prefix, "domain", tmp), "domain");
	}
	if ((value = getenv("wins")))
		nvram_set(strcat_r(prefix, "wins", tmp), trim_r(value));
	//if ((value = getenv("hostname")))
	//	sethostname(value, strlen(value) + 1);
	if ((value = getenv("lease"))) {
		unsigned int lease = atoi(value);
		nvram_set_int(strcat_r(prefix, "lease", tmp), lease);
		expires(wan_ifname, lease);
	}

	/* classful static routes */
	nvram_set(strcat_r(prefix, "routes", tmp), getenv("routes"));
	/* ms classless static routes */
	nvram_set(strcat_r(prefix, "routes_ms", tmp), getenv("msstaticroutes"));
	/* rfc3442 classless static routes */
	nvram_set(strcat_r(prefix, "routes_rfc", tmp), getenv("staticroutes"));

#ifdef RTCONFIG_IPV6
	if ((value = getenv("ip6rd")) &&
	    (get_ipv6_service_by_unit(ifunit) == IPV6_6RD && nvram_match(ipv6_nvname_by_unit("ipv6_6rd_dhcp", ifunit), "1"))) {
		char *ptr, *pvalue, *values[4];
		int i;

		ptr = pvalue = strdup(value);
		for (i = 0; pvalue && i < 4; i++)
			values[i] = strsep(&pvalue, " ");
		if (i == 4) {
			nvram_set(strcat_r(wanprefix, "6rd_ip4size", tmp), values[0]);
			nvram_set(strcat_r(wanprefix, "6rd_prefixlen", tmp), values[1]);
			nvram_set(strcat_r(wanprefix, "6rd_prefix", tmp), values[2]);
			nvram_set(strcat_r(wanprefix, "6rd_router", tmp), values[3]);
		}
		free(ptr);
	}
#endif

#ifdef RTCONFIG_TR069
	if ((value = getenv("opt43")) && nvram_get_int("tr_discovery") &&
	    (value = hex2bin(value, &size))) {
		struct opt_hdr *opt;
		char buf[256], *url = NULL, *userinfo, *host, *path, *ptr, *user, *pass;
		if ((opt = opt_get(value, size, 1)) &&
		    (ptr = strstr(stropt(opt, buf), "://")) && ptr > buf)
			url = trim_r(buf);
		else if ((ptr = strstr(value, "://")) && ptr > value)
			url = trim_r(value);
		if (url && (
		    strncmp(url, "http://", sizeof("http://") - 1) == 0 ||
		    strncmp(url, "https://", sizeof("https://") - 1) == 0)) {
			host = strtok_r(ptr + 3, " ", &userinfo);
			path = strchrnul(host, '/');
			if ((ptr = strchr(host, '@')) && ptr < path) {
				ptr = strsep(&host, "@");
				if ((userinfo = user = pass = strdup(ptr)))
					strsep(&pass, ":");
				url = memmove(host - (ptr - url), url, ptr - url);
			} else {
				user = strtok_r(NULL, " ", &userinfo);
				pass = strtok_r(NULL, " ", &userinfo);
				userinfo = NULL;
			}
			if (host < path) {
				if (user || pass) {
					//nvram_set(strcat_r(wanprefix, "tr_username", tmp), user ? : "");
					//nvram_set(strcat_r(wanprefix, "tr_passwd", tmp), pass ? : "");
					nvram_set("tr_username", user ? : "");
					nvram_set("tr_passwd", pass ? : "");
				}
				//nvram_set(strcat_r(wanprefix, "tr_acs_url", tmp), url);
				nvram_set("tr_acs_url", url);
				nvram_set_int("tr_enable", 1);
			}
			free(userinfo);
		}
		if ((opt = opt_get(value, size, 2))) {
			//nvram_set(strcat_r(wanprefix, "tr_pvgcode", tmp), stropt(opt, buf));
			nvram_set("pvgcode", stropt(opt, buf));
		}
		free(value);
	}
#ifdef RTCONFIG_TR181
	nvram_unset("vivso");
	if ((value = hex2bin(getenv("opt125"), &size))) {
		struct viopt_hdr *viopt;
		struct opt_hdr *oui, *serial, *class;
		if ((viopt = viopt_get(value, size, htonl(3561))) &&
		    (oui = opt_get(viopt->data, viopt->len, 4)) &&
		    (serial = opt_get(viopt->data, viopt->len, 5))) {
			char vivso[6 + 64 + 64 + 3];
			char *ptr = vivso;
			char *end = ptr + sizeof(vivso);
			ptr += snprintf(ptr, end - ptr, "%s,", stropt(oui, tmp));
			ptr += snprintf(ptr, end - ptr, "%s,", stropt(serial, tmp));
			if ((class = opt_get(viopt->data, viopt->len, 6)))
				ptr += snprintf(ptr, end - ptr, "%s", stropt(class, tmp));
			nvram_set("vivso", vivso);
		}
		free(value);
	}
#endif
#endif

#if defined(RTCONFIG_AMAS) && defined(RTCONFIG_PRELINK)
	if (nvram_match("x_Setting", "0")) {
		int verified = 0;
		if ((value = hex2bin(getenv("opt125"), &size))) {
			struct viopt_hdr *viopt;
			struct opt_hdr *hash_bundle_key_opt;
			if ((viopt = viopt_get(value, size, htonl(2623))) &&
			    (hash_bundle_key_opt = opt_get(viopt->data, viopt->len, 123)) && hash_bundle_key_opt->len == 20) {
				verified = amas_verify_hash_bundle_key(hash_bundle_key_opt->data, &verified) == AMAS_RESULT_SUCCESS &&
					   verified == 1;
			}
			free(value);
		}
		nvram_set("amas_bdl_wanstate", verified ? "1" : "2");
#if defined(RTCONFIG_BT_CONN)
		ble_rename_ssid();
#endif
	}
#endif

	// check if the ipaddr is safe to apply
	// only handle one lan instance so far
	// update_wan_state(prefix, WAN_STATE_STOPPED, WAN_STOPPED_REASON_INVALID_IPADDR)
	if (inet_equal(nvram_safe_get(strcat_r(prefix, "ipaddr", tmp)), nvram_safe_get(strcat_r(prefix, "netmask", tmp)),
		       nvram_safe_get("lan_ipaddr"), nvram_safe_get("lan_netmask"))) {
		update_wan_state(prefix, WAN_STATE_STOPPED, WAN_STOPPED_REASON_INVALID_IPADDR);
		return 0;
	}

#if defined(RTCONFIG_PORT_BASED_VLAN) || defined(RTCONFIG_TAGGED_BASED_VLAN)
	/* If return value of test_and_get_free_char_network() is 1 and
	 * we got different IP/netmask from it, the WAN IP/netmask conflicts with known networks.
	 */
	snprintf(ip_mask, sizeof(ip_mask), "%s/%s",
		nvram_pf_safe_get(prefix, "ipaddr"), nvram_pf_safe_get(prefix, "netmask"));
	if (test_and_get_free_char_network(7, ip_mask, EXCLUDE_NET_ALL_EXCEPT_LAN_VLAN) == 1) {
		logmessage("dhcp", "%s/%s conflicts with known networks",
			nvram_pf_safe_get(prefix, "ipaddr"), nvram_pf_safe_get(prefix, "netmask"));
		update_wan_state(prefix, WAN_STATE_STOPPED, WAN_STOPPED_REASON_INVALID_IPADDR);
		return 0;
	}
#endif
#if defined(RTCONFIG_COOVACHILLI)
	restart_coovachilli_if_conflicts(nvram_pf_get(prefix, "ipaddr"), nvram_pf_get(prefix, "netmask"));
#endif

	/* Clean nat conntrack for this interface,
	 * but skip physical VPN subinterface for PPTP/L2TP */
	if (changed && !(unit < 0 &&
	    (nvram_match(strcat_r(wanprefix, "proto", tmp), "l2tp") ||
	     nvram_match(strcat_r(wanprefix, "proto", tmp), "pptp"))))
		ifconfig(wan_ifname, IFUP, "0.0.0.0", NULL);
	ifconfig(wan_ifname, IFUP,
		 nvram_safe_get(strcat_r(prefix, "ipaddr", tmp)),
		 nvram_safe_get(strcat_r(prefix, "netmask", tmp)));

#ifdef RTCONFIG_ISP_CUSTOMIZE_TOOL
	if ((value = getenv("tftp"))) {
		customize_tool(value);
	}
#endif

	wan_up(wan_ifname);

	logmessage("dhcp client", "%s %s/%s via %s for %d seconds.",
		renew ? "renew" : "bound",
		nvram_safe_get(strcat_r(prefix, "ipaddr", tmp)),
		nvram_safe_get(strcat_r(prefix, "netmask", tmp)),
		nvram_safe_get(strcat_r(prefix, "gateway", tmp)),
		nvram_get_int(strcat_r(prefix, "lease", tmp)));

	_dprintf("udhcpc:: %s done\n", __FUNCTION__);
	return 0;
}

/*
 * renew: This argument is used when a DHCP lease is renewed. All of
 * the paramaters are set in enviromental variables. This argument is
 * used when the interface is already configured, so the IP address,
 * will not change, however, the other DHCP paramaters, such as the
 * default gateway, subnet mask, and dns server may change.
 */
static int
renew(void)
{
	char *wan_ifname = safe_getenv("interface");
	char *value;
	char tmp[100], prefix[sizeof("wanXXXXXXXXXX_")];
	char wanprefix[sizeof("wanXXXXXXXXXX_")];
	int unit, ifunit;
	int changed = 0;
#if defined(RTCONFIG_USB_MODEM) && defined(RTCONFIG_INTERNAL_GOBI)
	int modem_unit;
	char tmp2[100], prefix2[32];
#endif

	/* Figure out nvram variable name prefix for this i/f */
	if ((ifunit = wan_prefix(wan_ifname, wanprefix)) < 0)
		return -1;
	if ((unit = wan_ifunit(wan_ifname)) < 0)
		snprintf(prefix, sizeof(prefix), "wan%d_x", ifunit);
	else
		snprintf(prefix, sizeof(prefix), "wan%d_", ifunit);

	if (!nvram_check(strcat_r(prefix, "ipaddr", tmp), trim_r(getenv("ip"))))
		return bound(1);
#if defined(RTCONFIG_USB_MODEM) && defined(RTCONFIG_INTERNAL_GOBI)
	if (dualwan_unit__usbif(ifunit)) {
		modem_unit = get_modemunit_by_dev(wan_ifname);
		if (modem_unit == MODEM_UNIT_NONE) {
			_dprintf("%s: cannot get the modem unit!\n", __FUNCTION__);
			return -1;
		}

		usb_modem_prefix(modem_unit, prefix2, sizeof(prefix2));
	}

	if (dualwan_unit__usbif(ifunit) && nvram_match(strcat_r(prefix2, "act_type", tmp2), "gobi")) {
		if (!nvram_check(strcat_r(prefix, "netmask", tmp), "255.255.255.255"))
			return bound(1);
		if (!nvram_check(strcat_r(prefix, "gateway", tmp), trim_r(getenv("ip"))))
			return bound(1);
	} else
#endif
	{
		if (!nvram_check(strcat_r(prefix, "netmask", tmp), trim_r(getenv("subnet"))))
			return bound(1);
		if (!nvram_check(strcat_r(prefix, "gateway", tmp), trim_r(getenv("router"))))
			return bound(1);
	}

	if (nvram_get_int(strcat_r(wanprefix, "dnsenable_x", tmp))) {
		/* ex: android phone, the gateway is the DNS server. */
		if ((value = getenv("dns") ? : getenv("router")))
			changed += nvram_set_check(strcat_r(prefix, "dns", tmp), trim_r(value));
#ifdef DHCP_RFC3397
		if ((value = getenv("search")) && *value) {
			char *domain, *result;
			if ((domain = getenv("domain")) && *domain &&
			    find_word(value, trim_r(domain)) == NULL) {
				result = alloca(strlen(domain) + strlen(value) + 2);
				sprintf(result, "%s %s", domain, value);
				value = result;
			}
			changed += nvram_set_check(strcat_r(prefix, "domain", tmp), trim_r(value));
		} else
#endif
		changed += nvram_set_env(strcat_r(prefix, "domain", tmp), "domain");
	}
	if ((value = getenv("wins")))
		nvram_set(strcat_r(prefix, "wins", tmp), trim_r(value));
	//if ((value = getenv("hostname")))
	//	sethostname(value, strlen(value) + 1);
	if ((value = getenv("lease"))) {
		unsigned int lease = atoi(value);
		nvram_set_int(strcat_r(prefix, "lease", tmp), lease);
		expires(wan_ifname, lease);
	}

	/* Update actual DNS or delayed for DHCP+PPP */
	if (changed)
		update_resolvconf();

	/* Update connected state and DNS for WEB UI,
	 * but skip physical VPN subinterface */
	if (changed && !(unit < 0))
		update_wan_state(wanprefix, WAN_STATE_CONNECTED, 0);

	_dprintf("udhcpc:: %s done\n", __FUNCTION__);
	return 0;
}

static int
leasefail(void)
{
	char *wan_ifname = safe_getenv("interface");
	char tmp[100], prefix[sizeof("wanXXXXXXXXXX_")];
	char pid[sizeof("/var/run/zcipXXXXXXXXXX.pid")];
	int unit;

	/* Figure out nvram variable name prefix for this i/f */
	if ((unit = wan_prefix(wan_ifname, prefix)) < 0)
		return -1;

	/* Start zcip for pppoe only */
	if (!nvram_match(strcat_r(prefix, "proto", tmp), "pppoe"))
		return 0;

	snprintf(pid, sizeof(pid), "/var/run/zcip%d.pid", unit);
	if (kill_pidfile_s(pid, 0) == 0)
		return 0;

	return start_zcip(wan_ifname, unit, NULL);
}

int
udhcpc_wan(int argc, char **argv)
{
	run_custom_script("dhcpc-event", 0, argv[1], "4");

	if(argv[1] && !strstr(argv[1], "leasefail"))
		_dprintf("%s:: %s\n", __func__, argv[1]);
	if (!argv[1]){
		_dprintf("%s::\n", __func__);
		return EINVAL;
	}
	else if (strstr(argv[1], "deconfig"))
		return deconfig(0);
	else if (strstr(argv[1], "bound"))
		return bound(0);
	else if (strstr(argv[1], "renew"))
		return renew();
	else if (strstr(argv[1], "leasefail"))
		return leasefail();
/*	else if (strstr(argv[1], "nak")) */

	return 0;
}

int
start_udhcpc(char *wan_ifname, int unit, pid_t *ppid)
{
	char tmp[100], prefix[sizeof("wanXXXXXXXXXX_")];
	char pid[sizeof("/var/run/udhcpcXXXXXXXXXX.pid")];
	char clientid[sizeof("61:") + (128*2) + 1];
#ifdef RTCONFIG_TR069
	char vendorid[32+32+sizeof(" dslforum.org")];
#if defined(RTCONFIG_TR181) || defined(RTCONFIG_DSL)
	unsigned char optbuf[sizeof(struct viopt_hdr) + 128];
	unsigned char hwaddr[6];
	char vivopts[sizeof("125:") + sizeof(optbuf)*2];
#endif
#endif
	struct duid duid;
	char *value;
	char *dhcp_argv[] = { "/sbin/udhcpc",
		"-i", wan_ifname,
		"-p", (snprintf(pid, sizeof(pid), "/var/run/udhcpc%d.pid", unit), pid),
		"-s", "/tmp/udhcpc_wan",
		NULL,		/* -t2 */
		NULL,		/* -T5 */
		NULL,		/* -A120 */
		NULL,		/* -b */
		NULL, NULL,	/* -H/-F wan_hostname */
		NULL,		/* -Oroutes */
		NULL,		/* -Ostaticroutes */
		NULL,		/* -Omsstaticroutes */
#ifdef RTCONFIG_IPV6
		NULL,		/* -Oip6rd rfc */
		NULL,		/* -Oip6rd comcast */
#endif
		NULL, NULL,	/* -x 61:wan_clientid */
		NULL, NULL,	/* -V wan_vendorid */
#ifdef RTCONFIG_TR069
		NULL,		/* -O43 */
#if defined(RTCONFIG_TR181) || defined(RTCONFIG_DSL)
		NULL, NULL,	/* -x 125:vivopts */
#endif
#endif
		NULL, NULL,	/* -x 61:wan_clientid (non-DSL) */
		NULL };
	int index = 7;		/* first NULL */
	int len, dr_enable;
	int dhcp_qry;

	/* Use unit */
	snprintf(prefix, sizeof(prefix), "wan%d_", unit);

	/* Stop zcip to avoid races */
	stop_zcip(unit);

	/* Skip dhcp and start zcip for pppoe, if desired */
	if ((nvram_match(strcat_r(prefix, "proto", tmp), "pppoe") &&
	    nvram_match(strcat_r(prefix, "vpndhcp", tmp), "0"))
#if defined(RTCONFIG_IPV6) && (defined(RTAX82_XD6) || defined(RTAX82_XD6S) || defined(XD6_V2) || defined(ET12))
	||  (!strncmp(nvram_safe_get("territory_code"), "CH", 2) &&
	    ipv6_enabled() && nvram_match(ipv6_nvname("ipv6_only"), "1"))
#endif
	)
		return start_zcip(wan_ifname, unit, ppid);

	/* DHCP query frequency */
	value = nvram_get(strcat_r(prefix, "dhcp_qry", tmp)); // new nvram with wan index
	if (value && strlen(value))
		dhcp_qry = atoi(value);
	else
		dhcp_qry = nvram_get_int("dhcpc_mode");	// default = Aggressive mode
	if (dhcp_qry == 0) {	// Normal mode
		/* 2 discover packets max (default 3 discover packets) */
		dhcp_argv[index++] = "-t2";
		/* 5 seconds between packets (default 3 seconds) */
		dhcp_argv[index++] = "-T5";
		/* Wait 160 seconds before trying again (default 20 seconds) */
		/* set to 160 to accomodate new timings enforced by Charter cable */
		dhcp_argv[index++] = "-A160";
	} else if(dhcp_qry == 1){	// Aggressive mode
		dhcp_argv[index++] = "-A5";
	} else if(dhcp_qry == 2){	// Continuous mode
		dhcp_argv[index++] = "-t1";
		dhcp_argv[index++] = "-T5";
		dhcp_argv[index++] = "-A0";
	}

	if (ppid == NULL)
		dhcp_argv[index++] = "-b";

	value = nvram_safe_get(strcat_r(prefix, "hostname", tmp));
	if (*value) {
		char *fqdn = strchr(value, '.');
		if (fqdn ? is_valid_domainname(value) : is_valid_hostname(value)) {
			dhcp_argv[index++] = fqdn ? "-F" : "-H";
			dhcp_argv[index++] = value;
		}
	}

	/* 0: disable, 1: MS routes, 2: RFC routes, 3: Both */
	dr_enable = nvram_get_int("dr_enable_x");
	if (dr_enable != 0) {
		dhcp_argv[index++] = "-O33";		/* routes */
		if (dr_enable & (1 << 0))
			dhcp_argv[index++] = "-O249";	/* "msstaticroutes" */
		if (dr_enable & (1 << 1))
			dhcp_argv[index++] = "-O121";	/* "staticroutes" */
	}

#ifdef RTCONFIG_IPV6
	if (get_ipv6_service_by_unit(unit) == IPV6_6RD && nvram_match(ipv6_nvname_by_unit("ipv6_6rd_dhcp", unit), "1")) {
		dhcp_argv[index++] = "-O212";		/* ip6rd rfc */
		dhcp_argv[index++] = "-O150";		/* ip6rd comcast */
	}
#endif

	/* Client ID */
	value = nvram_safe_get(strcat_r(prefix, "clientid", tmp));
	if (nvram_get_int(strcat_r(prefix, "clientid_type", tmp))) {
		if (get_duid(&duid)) {
			/* RFC4361 implementation, use WAN number as IAID.
			 * This also fits odhcp6c behavior for IA_NA for the WAN0 */
			unsigned long iaid = htonl(unit + 1);
			len = snprintf(clientid, sizeof(clientid), "61:ff");
			len += bin2hex(clientid + len, sizeof(clientid) - len, &iaid, sizeof(iaid));
			bin2hex(clientid + len, sizeof(clientid) - len, &duid, sizeof(duid));
			dhcp_argv[index++] = "-x";
			dhcp_argv[index++] = clientid;
		}
	} else if (*value) {
		len = snprintf(clientid, sizeof(clientid), "61:");
		bin2hex(clientid + len, sizeof(clientid) - len, value, strlen(value));
		dhcp_argv[index++] = "-x";
		dhcp_argv[index++] = clientid;
	}

	/* Vendor ID */
	value = nvram_safe_get(strcat_r(prefix, "vendorid", tmp));
#ifdef RTCONFIG_TR069
	if (nvram_get_int("tr_discovery")) {
		/* Add dslforum.org to VCI and request VSI */
		value = stpncpy(vendorid, *value ? value : "udhcp",
				sizeof(vendorid) - sizeof(" dslforum.org"));
		snprintf(value, sizeof(vendorid) - (value - vendorid), " %s", "dslforum.org");
		value = vendorid;
		dhcp_argv[index++] = "-O43";
	}
#endif
	if (*value) {
		dhcp_argv[index++] = "-V";
		dhcp_argv[index++] = value;
	}

#ifdef RTCONFIG_TR069
#if defined(RTCONFIG_TR181) || defined(RTCONFIG_DSL)
	if (ether_atoe(get_lan_hwaddr(), hwaddr)) {
		struct viopt_hdr *viopt = (struct viopt_hdr *) optbuf;
		unsigned char *ptr = viopt->data;
		unsigned char *end = optbuf + sizeof(optbuf);

		/* OUI */
		len = snprintf(tmp, sizeof(tmp), "%02X%02X%02X", hwaddr[0], hwaddr[1], hwaddr[2]);
		ptr += opt_add(ptr, end - ptr, 1, tmp, len);
		/* Serial */
		len = snprintf(tmp, sizeof(tmp), "%02X%02X%02X%02X%02X%02X",
			       hwaddr[0], hwaddr[1], hwaddr[2], hwaddr[3], hwaddr[4], hwaddr[5]);
		ptr += opt_add(ptr, end - ptr, 2, tmp, len);
		/* Class */
		len = snprintf(tmp, sizeof(tmp), "%s", nvram_safe_get("productid"));
		ptr += opt_add(ptr, end - ptr, 3, tmp, len);

		viopt->entnum = htonl(3561);
		viopt->len = ptr - viopt->data;

		len = snprintf(vivopts, sizeof(vivopts), "125:");
		bin2hex(vivopts + len, sizeof(vivopts) - len, viopt, sizeof(*viopt) + viopt->len);

		dhcp_argv[index++] = "-x";
		dhcp_argv[index++] = vivopts;
	}
#endif
#endif

	return _eval(dhcp_argv, NULL, 0, ppid);
}

void
stop_udhcpc(int unit)
{
	char pid[sizeof("/var/run/udhcpcXXXXXXXXXX.pid")];

	/* Stop zcip before udhcpc to avoid races */
	stop_zcip(unit);

	/* Stop all instances */
	if (unit < 0) {
		killall_tk("udhcpc");
		return;
	}

	/* Stop unit instance only */
	snprintf(pid, sizeof(pid), "/var/run/udhcpc%d.pid", unit);
	if (kill_pidfile_s(pid, SIGUSR2) == 0) {
		usleep(10000);
		kill_pidfile_tk(pid);
	}
}

/*
 * config: This argument is used when zcip moves to configured state.
 * All of the paramaters are set in enviromental variables, the script
 * should configure the interface.
*/
static int
config(void)
{
	char *wan_ifname = safe_getenv("interface");
	char tmp[100], prefix[sizeof("wanXXXXXXXXXX_")];
	char wanprefix[sizeof("wanXXXXXXXXXX_")];
	int unit, ifunit;
	int changed = 0;

	/* Figure out nvram variable name prefix for this i/f */
	if ((ifunit = wan_prefix(wan_ifname, wanprefix)) < 0)
		return -1;
	if ((unit = wan_ifunit(wan_ifname)) < 0)
		snprintf(prefix, sizeof(prefix), "wan%d_x", ifunit);
	else	snprintf(prefix, sizeof(prefix), "wan%d_", ifunit);

	changed += nvram_set_env(strcat_r(prefix, "ipaddr", tmp), "ip");
	changed += nvram_set_check(strcat_r(prefix, "netmask", tmp), "255.255.0.0");
	changed += nvram_set_check(strcat_r(prefix, "gateway", tmp), "");
	if (nvram_get_int(strcat_r(wanprefix, "dnsenable_x", tmp)))
		nvram_set(strcat_r(prefix, "dns", tmp), "");
	nvram_unset(strcat_r(prefix, "wins", tmp));
	nvram_unset(strcat_r(prefix, "domain", tmp));
	nvram_unset(strcat_r(prefix, "lease", tmp));
	nvram_unset(strcat_r(prefix, "expires", tmp));

	nvram_unset(strcat_r(prefix, "routes", tmp));
	nvram_unset(strcat_r(prefix, "routes_ms", tmp));
	nvram_unset(strcat_r(prefix, "routes_rfc", tmp));

#ifdef RTCONFIG_TR069
//	nvram_unset(strcat_r(prefix, "tr_acs_url", tmp));
//	nvram_unset(strcat_r(prefix, "tr_pvgcode", tmp));
#ifdef RTCONFIG_TR181
	nvram_unset("vivso");
#endif
#endif

	/* Clean nat conntrack for this interface,
	 * but skip physical VPN subinterface for PPTP/L2TP */
	if (changed && !(unit < 0 &&
	    (nvram_match(strcat_r(wanprefix, "proto", tmp), "l2tp") ||
	     nvram_match(strcat_r(wanprefix, "proto", tmp), "pptp"))))
		ifconfig(wan_ifname, IFUP, "0.0.0.0", NULL);

	ifconfig(wan_ifname, IFUP,
		 nvram_safe_get(strcat_r(prefix, "ipaddr", tmp)),
		 nvram_safe_get(strcat_r(prefix, "netmask", tmp)));

	wan_up(wan_ifname);

#if defined(RTCONFIG_IPV6) && (defined(RTAX82_XD6) || defined(RTAX82_XD6S) || defined(XD6_V2) || defined(ET12))
	if ((!strncmp(nvram_safe_get("territory_code"), "CH", 2) &&
		ipv6_enabled() &&
		nvram_match(ipv6_nvname("ipv6_only"), "1")) &&
		(nvram_match(strcat_r(wanprefix, "proto", tmp), "dhcp") ||
		 nvram_match(strcat_r(wanprefix, "proto", tmp), "static") ||
		 nvram_match(strcat_r(wanprefix, "proto", tmp), "pppoe")))
		doSystem("route add default %s", wan_ifname);
#endif

	logmessage("zcip client", "configured %s",
		nvram_safe_get(strcat_r(prefix, "ipaddr", tmp)));

	_dprintf("zcip:: %s done\n", __FUNCTION__);
	return 0;
}

int
zcip_wan(int argc, char **argv)
{
	_dprintf("%s:: %s\n", __FUNCTION__, argv[1] ? : "");

        run_custom_script("zcip-event", 0, argv[1], NULL);

	if (!argv[1])
		return EINVAL;
	else if (strstr(argv[1], "deconfig"))
		return deconfig(1);
	else if (strstr(argv[1], "config"))
		return config();
/*	else if (strstr(argv[1], "init")) */

	return 0;
}

int
start_zcip(char *wan_ifname, int unit, pid_t *ppid)
{
	char pid[sizeof("/var/run/zcipXXXXXXXXXX.pid")];
	char *zcip_argv[] = { "/sbin/zcip",
		"-p", (snprintf(pid, sizeof(pid), "/var/run/zcip%d.pid", unit), pid),
		wan_ifname,
		"/tmp/zcip",
		NULL };

	return _eval(zcip_argv, NULL, 0, ppid);
}

void
stop_zcip(int unit)
{
	char pid[sizeof("/var/run/zcipXXXXXXXXXX.pid")];

	/* Stop all instances */
	if (unit < 0) {
		killall_tk("zcip");
		return;
	}

	/* Stop unit instance only */
	snprintf(pid, sizeof(pid), "/var/run/zcip%d.pid", unit);
	kill_pidfile_s(pid, SIGTERM);
}

static int
expires_lan(char *lan_ifname, unsigned int in)
{
	time_t now;
	FILE *fp;
	char tmp[100];

	time(&now);
	snprintf(tmp, sizeof(tmp), "/tmp/udhcpc-%s.expires", lan_ifname);
	if (!(fp = fopen(tmp, "w"))) {
		perror(tmp);
		return errno;
	}
	fprintf(fp, "%d", (unsigned int) now + in);
	fclose(fp);
	return 0;
}

#ifdef RTCONFIG_AMAS_WGN
static void restart_re_qos()
{
	// AMAS RE mode
	if (nvram_get_int("re_mode") == 1) start_iQos();
}
#endif

/* 
 * deconfig: This argument is used when udhcpc starts, and when a
 * leases is lost. The script should put the interface in an up, but
 * deconfigured state.
*/
static int
deconfig_lan(void)
{
	char *lan_ifname = safe_getenv("interface");

	//ifconfig(lan_ifname, IFUP, "0.0.0.0", NULL);
_dprintf("%s: IFUP.\n", __FUNCTION__);
#ifdef RTCONFIG_DHCP_OVERRIDE
	if (access_point_mode())
		;
	else
#endif
	if (nvram_match("lan_proto", "static"))
		ifconfig(lan_ifname, IFUP | IFF_ALLMULTI, nvram_safe_get("lan_ipaddr"), nvram_safe_get("lan_netmask"));
	else {
		nvram_set("lan_ipaddr", nvram_default_get("lan_ipaddr"));
		nvram_set("lan_netmask", nvram_default_get("lan_netmask"));
		nvram_set("lan_gateway", nvram_default_get("lan_gateway"));
		nvram_set("lan_lease", nvram_default_get("lan_lease"));
		nvram_set("lan_dns", nvram_default_get("lan_dns"));

		ifconfig(lan_ifname, IFUP | IFF_ALLMULTI, nvram_default_get("lan_ipaddr"), nvram_default_get("lan_netmask"));
	}

	expires_lan(lan_ifname, 0);

	lan_down(lan_ifname);

	_dprintf("done\n");
	return 0;
}

/*
 * bound: This argument is used when udhcpc moves from an unbound, to
 * a bound state. All of the paramaters are set in enviromental
 * variables, The script should configure the interface, and set any
 * other relavent parameters (default gateway, dns server, etc).
*/
static int
bound_lan(void)
{
	char *lan_ifname = safe_getenv("interface");
	char *value;
#if defined(RTCONFIG_TR069) && defined(RTCONFIG_TR181)
	char tmp[100];
	int size;
#endif
	int lanchange = 0;
	const char *ipaddr;


	if ((value = getenv("ip"))) {
		/* restart httpd after lan_ipaddr udpating through lan dhcp client */
		if (!nvram_match("lan_ipaddr", trim_r(value))) {
#if defined(HND_ROUTER) && defined(MCPD_PROXY)
			stop_mcpd_proxy();
			start_mcpd_proxy();
#endif
			stop_httpd();
			start_httpd();
			lanchange = 1;
		}
		nvram_set("lan_ipaddr", trim_r(value));
	}
	if ((value = getenv("subnet"))) {
		if (!nvram_match("lan_netmask", trim_r(value))) {
			lanchange = 1;
		}
		nvram_set("lan_netmask", trim_r(value));		
	}
	if ((value = getenv("router"))) {
		if (!nvram_match("lan_gateway", trim_r(value))) {
			lanchange = 1;
		}
		nvram_set("lan_gateway", trim_r(value));
	}
	if ((value = getenv("lease"))) {
		if (!nvram_match("lan_lease", trim_r(value))) {
			lanchange = 1;
		}
		nvram_set("lan_lease", trim_r(value));
		expires_lan(lan_ifname, atoi(value));
	}
	if (nvram_get_int("lan_dnsenable_x") && (value = getenv("dns"))) {
		if (!nvram_match("lan_dns", trim_r(value))) {
			lanchange = 1;
		}
		nvram_set("lan_dns", trim_r(value));
	}

#if defined(RTCONFIG_TR069) && defined(RTCONFIG_TR181)
	nvram_unset("vivso");
	if ((value = hex2bin(getenv("opt125"), &size))) {
		struct viopt_hdr *viopt;
		struct opt_hdr *oui, *serial, *class;
		if ((viopt = viopt_get(value, size, htonl(3561))) &&
		    (oui = opt_get(viopt->data, viopt->len, 4)) &&
		    (serial = opt_get(viopt->data, viopt->len, 5))) {
			char vivso[6 + 64 + 64 + 3];
			char *ptr = vivso;
			char *end = ptr + sizeof(vivso);
			ptr += snprintf(ptr, end - ptr, "%s,", stropt(oui, tmp));
			ptr += snprintf(ptr, end - ptr, "%s,", stropt(serial, tmp));
			if ((class = opt_get(viopt->data, viopt->len, 6)))
				ptr += snprintf(ptr, end - ptr, "%s", stropt(class, tmp));
			nvram_set("vivso", vivso);
		}
		free(value);
	}
#endif

_dprintf("%s: IFUP.\n", __FUNCTION__);

#ifdef RTCONFIG_AMAS_WGN
	/* move qos restart here to trigger early */
	restart_re_qos();
#endif

	ipaddr = getifaddr(lan_ifname, AF_INET, 0);
	if (ipaddr != NULL && (sw_mode() == SW_MODE_AP) && nvram_match("lan_ipaddr", (char*) ipaddr) && lanchange == 0 && nvram_get_int("lan_state_t") == LAN_STATE_CONNECTED) {
		return 0;
	}

	if ((repeater_mode()
#ifdef RTCONFIG_DPSTA
		|| (dpsta_mode() && nvram_get_int("re_mode") == 0)
#endif
		|| (rp_mode() && nvram_get_int("re_mode") == 0)
#if defined(RTCONFIG_BCMWL6) && defined(RTCONFIG_PROXYSTA)
		|| psr_mode() || mediabridge_mode()
#elif defined(RTCONFIG_REALTEK) || defined(RTCONFIG_QCA)
		|| (mediabridge_mode())
#endif
	     ) && nvram_get_int("wlc_mode") == 0) {
		update_lan_state(LAN_STATE_CONNECTED, 0);
		_dprintf("%s: done\n", __FUNCTION__);
		return 0;
	}

#ifdef RTCONFIG_DHCP_OVERRIDE
	if (sw_mode() == SW_MODE_AP && nvram_match("dnsqmode", "2")
#ifdef RTCONFIG_REALTEK
/* [MUST]: Need to Clarify */
	&& nvram_get_int("wlc_psta") == 0
#endif
	)
	{
		nvram_set("dnsqmode", "1");
		restart_dnsmasq(1);
	}
#endif

	ifconfig(lan_ifname, IFUP | IFF_ALLMULTI, nvram_safe_get("lan_ipaddr"),
		nvram_safe_get("lan_netmask"));

	lan_up(lan_ifname);

	_dprintf("%s: done\n", __FUNCTION__);
	return 0;
}

/*
 * renew: This argument is used when a DHCP lease is renewed. All of
 * the paramaters are set in enviromental variables. This argument is
 * used when the interface is already configured, so the IP address,
 * will not change, however, the other DHCP paramaters, such as the
 * default gateway, subnet mask, and dns server may change.
 */
static int
renew_lan(void)
{
	bound_lan();

	_dprintf("%s: done\n", __FUNCTION__);
	return 0;
}

/* dhcp client script entry for LAN (AP) */
int
udhcpc_lan(int argc, char **argv)
{
	_dprintf("%s:: %s\n", __FUNCTION__, argv[1] ? : "");

        run_custom_script("dhcpc-event", 0, argv[1], NULL);

	if (!argv[1])
		return EINVAL;
	else if (strstr(argv[1], "deconfig")) {
#if defined(RTCONFIG_AMAS)
		if (nvram_get_int("re_mode") == 1)
			logmessage("dhcp client", "deconfig");
#endif
		return deconfig_lan();
	}
	else if (strstr(argv[1], "bound"))
		return bound_lan();
	else if (strstr(argv[1], "renew"))
		return renew_lan();
#if defined(RTCONFIG_AMAS)
	else if (strstr(argv[1], "leasefail")) {
		if (nvram_get_int("re_mode") == 1)
			logmessage("dhcp client", "leasefail");
	}
#endif
/*	else if (strstr(argv[1], "nak")) */

	return EINVAL;
}

// -----------------------------------------------------------------------------

#ifdef RTCONFIG_IPV6
static int
deconfig6(char *wan_ifname, const int mode)
{
	char *lan_ifname = nvram_safe_get("lan_ifname");
	int unit = wan_ifunit(wan_ifname);
#ifdef RTCONFIG_SOFTWIRE46
	char wan_prefix[16];
	snprintf(wan_prefix, sizeof(wan_prefix), "wan%d_", unit);
#endif

	if (nvram_invmatch(ipv6_nvname_by_unit("ipv6_wan_addr", unit), "")) {
		eval("ip", "-6", "addr", "del", nvram_safe_get(ipv6_nvname_by_unit("ipv6_wan_addr", unit)), "dev", wan_ifname);
		nvram_set(ipv6_nvname_by_unit("ipv6_wan_addr", unit), "");
	}

	if (get_ipv6_service_by_unit(unit) == IPV6_NATIVE_DHCP &&
		nvram_get_int(ipv6_nvname_by_unit("ipv6_dhcp_pd", unit))) {
		if (nvram_invmatch(ipv6_nvname_by_unit("ipv6_prefix", unit), "") ||
			nvram_get_int(ipv6_nvname_by_unit("ipv6_prefix_length", unit)) != 0) {
			eval("ip", "-6", "addr", "flush", "scope", "global", "dev", lan_ifname);
			nvram_set(ipv6_nvname_by_unit("ipv6_rtr_addr", unit), "");
			nvram_set(ipv6_nvname_by_unit("ipv6_prefix", unit), "");
			nvram_set(ipv6_nvname_by_unit("ipv6_prefix_length", unit), "");
		}
#ifdef RTCONFIG_SOFTWIRE46
		if (nvram_invmatch(ipv6_nvname_by_unit("ipv6_ra_prefix", unit), "") ||
			nvram_get_int(ipv6_nvname_by_unit("ipv6_ra_prefix_length", unit)) != 0) {
			eval("ip", "-6", "addr", "flush", "scope", "global", "dev", lan_ifname);
			nvram_set(ipv6_nvname_by_unit("ipv6_ra_prefix", unit), "");
			nvram_set(ipv6_nvname_by_unit("ipv6_ra_prefix_length", unit), "");
		}
#endif
	}

	if (nvram_invmatch(ipv6_nvname_by_unit("ipv6_get_dns", unit), "") ||
	    nvram_invmatch(ipv6_nvname_by_unit("ipv6_get_domain", unit), "")) {
		nvram_set(ipv6_nvname_by_unit("ipv6_get_dns", unit), "");
		nvram_set(ipv6_nvname_by_unit("ipv6_get_domain", unit), "");
		if (nvram_get_int(ipv6_nvname_by_unit("ipv6_dnsenable", unit)))
			update_resolvconf();
	}

#ifdef RTCONFIG_SOFTWIRE46
	switch (get_ipv4_service_by_unit(unit)) {
	case WAN_LW4O6:
	case WAN_MAPE:
		stop_s46_tunnel(unit, 1);
		S46_DBG("STOP_S46_TUNNEL\n");
		break;
	case WAN_V6PLUS:
	case WAN_OCNVC:
		if (nvram_pf_get_int(wan_prefix, "s46_hgw_case") != S46_CASE_MAP_HGW_ON) {
			stop_s46_tunnel(unit, 1);
			S46_DBG("STOP_S46_TUNNEL\n");
		}
		break;
	case WAN_DSLITE:
		nvram_set(ipv6_nvname_by_unit("ipv6_s46_env_aftr", unit), "");
		stop_s46_tunnel(unit, 1);
		S46_DBG("STOP_S46_TUNNEL\n");
		break;
	}

	if (nvram_match("x_Setting", "0") && !strncmp(nvram_safe_get("territory_code"), "JP", 2)) {
		nvram_set(ipv6_nvname_by_unit("ipv6_ra_addr", unit), "");
		nvram_set(ipv6_nvname_by_unit("ipv6_ra_length", unit), "");
	}
#endif
#ifdef RTCONFIG_INADYN
	if(mode == 1)
	{
		if (nvram_get_int("ddns_enable_x") && nvram_get_int("ddns_ipv6_update")) {
			stop_ddns();
			start_ddns(NULL);
		}
	}
#endif
	return 0;
}

#ifdef RTCONFIG_SOFTWIRE46
static int bmemcmp(const void *s1, const void *s2, size_t bits)
{
	size_t bytes = bits / 8;
	bits %= 8;

	int ret = memcmp(s1, s2, bytes);
	if (ret == 0 && bits > 0) {
		const uint8_t *a = s1, *b = s2;
		ret = (a[bytes] >> (8 - bits)) - (b[bytes] >> (8 - bits));
	}

	return ret;
}

static void *bmemcpy(void *dest, const void *src, size_t bits)
{

	size_t bytes = bits / 8;
	bits %= 8;

	memcpy(dest, src, bytes);
	if (bits > 0) {
		uint8_t *a = dest;
		const uint8_t *b = src;
		uint8_t mask = (1 << (8 - bits)) - 1;
		a[bytes] = (a[bytes] & mask) | (~mask & b[bytes]);
	}

	return dest;
}

static void *bmemcpys64(void *dest, const void *src, size_t frombits, size_t nbits)
{
	uint64_t buf = 0;
	const uint8_t *b = src;
	
	size_t frombyte = frombits / 8, tobyte = (frombits + nbits) / 8;
	frombits %= 8;

	memcpy(&buf, &b[frombyte], tobyte - frombyte + 1);
	if (frombits > 0)
		buf = __cpu_to_be64(__be64_to_cpu(buf) << frombits);

	return bmemcpy(dest, &buf, nbits);
}

static void s46_match_prefix(struct in6_addr *pd, int *pdlen,
	const char *subnet,
	const struct in6_addr *prefix6, int prefix6len, int lw4o6)
{
	char addrbuf[INET6_ADDRSTRLEN];
	struct in6_addr prefix;
	int mask = 128;

	if (sscanf(subnet, "%45[^/]/%d", addrbuf, &mask) < 1 ||
	    inet_pton(AF_INET6, addrbuf, &prefix) <= 0)
		return;

	if (lw4o6 && mask == 128)
		mask = 64;

	if (*pdlen < mask && mask >= prefix6len &&
	    !bmemcmp(&prefix, prefix6, prefix6len)) {
		bmemcpy(pd, &prefix, mask);
		*pdlen = mask;
	} else
	if (lw4o6 && *pdlen < prefix6len && mask < prefix6len &&
	    !bmemcmp(&prefix, prefix6, mask)) {
		bmemcpy(pd, prefix6, prefix6len);
		*pdlen = prefix6len;
	}
}

int s46_mapcalc(int wan_unit, int wan_proto, char *rules, char *peerbuf, size_t peerbufsz,
	char *addr6buf, size_t addr6bufsz, char *addr4buf, size_t addr4bufsz,
	int *poffset, int *ppsidlen, int *ppsid, char **fmrs, int draft)
{
	FILE *fp, *mapfp;
	char *rule, *next_rule, *item, *next, *name, *value, *fmrbuf;
	char v6maps[20];
	size_t fmrbufsz;
	int ret = 0;

	if (!rules || *rules == '\0' || (rules = strdup(rules)) == NULL)
		return -1;

	snprintf(v6maps, sizeof(v6maps), S46_MAP_PATH, wan_unit);
	mapfp = fopen(v6maps, "w");

	fmrbuf = NULL;
	fp = fmrs ? open_memstream(&fmrbuf, &fmrbufsz) : NULL;
	for (rule = strtok_r(rules, " ", &next_rule); rule; rule = strtok_r(NULL, " ", &next_rule)) {
		char addrbuf[INET6_ADDRSTRLEN + sizeof("/128")];
		char addrbuf6[INET6_ADDRSTRLEN + sizeof("/128")];
		struct in6_addr peer = IN6ADDR_ANY_INIT;
		struct in_addr addr4 = { INADDR_ANY };
		struct in6_addr addr6 = IN6ADDR_ANY_INIT;
		struct in6_addr pd = IN6ADDR_ANY_INIT;
		struct in_addr prefix4 = { INADDR_ANY };
		struct in6_addr prefix6 = IN6ADDR_ANY_INIT;
		int addr4len = 32, pdlen = -1;
		int prefix4len = 32, prefix6len = -1;
		int ealen = -1, offset = -1, psidlen = -1, psid = -1;
		int lw4o6 = 0, fmr = 0, err = 0;
		uint16_t psid16 = 0;

		S46_DBG("[rule]:[%s]\n", rule);
		for (item = strtok_r(rule, ",", &next); item; item = strtok_r(NULL, ",", &next)) {
			value = item;
			name = strsep(&value, "=");
			if (strcmp(name, "fmr") == 0)
				fmr = 1;
			else if (value == NULL)
				err = 1;
			else if (strcmp(name, "type") == 0)
				lw4o6 = (value && strcmp(value, "lw4o6") == 0);
			else if (strcmp(name, "ipv4prefix") == 0)
				err = !inet_pton(AF_INET, value, &prefix4);
			else if (strcmp(name, "prefix4len") == 0)
				err = (prefix4len = strtoul(value, NULL, 0)) > 32;
			else if (strcmp(name, "ipv6prefix") == 0)
				err = !inet_pton(AF_INET6, value, &prefix6);
			else if (strcmp(name, "prefix6len") == 0)
				err = (prefix6len = strtoul(value, NULL, 0)) > 128;
			else if (strcmp(name, "ealen") == 0)
				err = (ealen = strtoul(value, NULL, 0)) > 48;
			else if (strcmp(name, "psidlen") == 0)
				err = (psidlen = strtoul(value, NULL, 0)) > 16;
			else if (strcmp(name, "offset") == 0)
				err = (offset = strtoul(value, NULL, 0)) > 16;
			else if (strcmp(name, "psid") == 0)
				err = (psid = strtoul(value, NULL, 0)) > 65535;
			else if (strcmp(name, "br") == 0)
				err = !inet_pton(AF_INET6, value, &peer);
			else if (strcmp(name, "pd") == 0)
				err = !inet_pton(AF_INET6, value, &pd);
			else if (strcmp(name, "pdlen") == 0)
				err = (pdlen = strtoul(value, NULL, 0)) > 128;
			if (err)
				break;
		}
		if (err || prefix4len < 0 || prefix6len < 0)
			continue;

		if (offset < 0)
			offset = lw4o6 ? 0 : (draft ? 4 : 6);

		if (lw4o6) {
			if (psidlen < 0)
				psidlen = 0;
			ealen = psidlen;
		}

		if (pdlen < 0) {
			switch (wan_proto) {
				char addrbuf[INET6_ADDRSTRLEN + sizeof("/128")];
				char tmpbuf[INET6_ADDRSTRLEN + sizeof("/128")];
			case WAN_LW4O6:
			case WAN_MAPE:
				snprintf(addrbuf, sizeof(addrbuf), "%s/%d",
					inet_ntop(AF_INET6, &prefix6, tmpbuf, sizeof(tmpbuf)), prefix6len);
				s46_match_prefix(&pd, &pdlen, addrbuf, &prefix6, prefix6len, lw4o6);
				S46_DBG("[addrbuf=%s] [pd=%s] [pdlen=%d]\n",
					addrbuf, inet_ntop(AF_INET6, &pd, tmpbuf, sizeof(tmpbuf)), pdlen);
				break;
			case WAN_V6PLUS:
			case WAN_OCNVC:
				if (nvram_invmatch(ipv6_nvname_by_unit("ipv6_prefix", wan_unit), "") &&
				    nvram_invmatch(ipv6_nvname_by_unit("ipv6_prefix_length", wan_unit), "")) {
					snprintf(addrbuf, sizeof(addrbuf), "%s/%d",
						 nvram_safe_get(ipv6_nvname_by_unit("ipv6_prefix", wan_unit)),
						 nvram_get_int(ipv6_nvname_by_unit("ipv6_prefix_length", wan_unit)));
					s46_match_prefix(&pd, &pdlen, addrbuf, &prefix6, prefix6len, lw4o6);
					S46_DBG("[addrbuf=%s] [pdlen=%d]\n", addrbuf, pdlen);
				} else {
					if (nvram_invmatch(ipv6_nvname_by_unit("ipv6_ra_addr", wan_unit), "") &&
					    nvram_invmatch(ipv6_nvname_by_unit("ipv6_ra_length", wan_unit), "")) {
						snprintf(addrbuf, sizeof(addrbuf), "%s/%d",
							nvram_safe_get(ipv6_nvname_by_unit("ipv6_ra_addr", wan_unit)),
							nvram_get_int(ipv6_nvname_by_unit("ipv6_ra_length", wan_unit)));
						s46_match_prefix(&pd, &pdlen, addrbuf, &prefix6, prefix6len, lw4o6);
					}
				}
				break;
			}
		}

		if (ealen < 0 && pdlen >= 0)
			ealen = pdlen - prefix6len;

		if (psidlen <= 0) {
			psidlen = ealen - (32 - prefix4len);
			if (psidlen < 0)
				psidlen = 0;
			psid = -1;
		}

		if (ealen < 0 || psidlen > 16 || ealen < psidlen) {
			S46_DBG("[skip] ealen(%d) < 0 || psidlen(%d) > 16 || ealen(%d) < psidlen(%d)\n",
				ealen, psidlen, ealen, psidlen);
			continue;
		}

		if (psid < 0 && psidlen >= 0 && pdlen >= 0) {
			bmemcpys64(&psid16, &pd, prefix6len + ealen - psidlen, psidlen);
			psid = ntohs(psid16) >> (16 - psidlen);
			S46_DBG("[psid]:%d\n", psid);
		} else if (psidlen >= 0) {
			psid &= 0xffff >> (16 - psidlen);
			S46_DBG("[psid]:%d\n", psid);
		}

		if (pdlen >= 0 || ealen == psidlen) {
			bmemcpys64(&addr4, &pd, prefix6len, ealen - psidlen);
			addr4.s_addr = htonl(ntohl(addr4.s_addr) >> prefix4len);
			bmemcpy(&addr4, &prefix4, prefix4len);
			if (prefix4len + ealen < 32)
				addr4len = prefix4len + ealen;
		}

		if (pdlen < 0 && !fmr)
			continue;

		if (pdlen >= 0) {
			int i = draft ? 9 : 10;
			psid16 = htons(psid);
			memcpy(&addr6.s6_addr[i], &addr4, sizeof(addr4));
			memcpy(&addr6.s6_addr[i + 4], &psid16, sizeof(psid16));
			bmemcpy(&addr6, &pd, pdlen);
			inet_ntop(AF_INET6, &peer, peerbuf, peerbufsz);
			inet_ntop(AF_INET6, &addr6, addr6buf, addr6bufsz);
			inet_ntop(AF_INET, &addr4, addrbuf, sizeof(addrbuf));
			snprintf(addr4buf, addr4bufsz, "%s/%d", addrbuf, addr4len);
			*poffset = offset;
			*ppsidlen = psidlen;
			*ppsid = psid;
			S46_DBG("[PeerAddr=%s]\n", peerbuf);
			S46_DBG("[IPv6Addr=%s]\n", addr6buf);
			S46_DBG("[IPv4Addr=%s]\n", addrbuf);
			S46_DBG("[PSIDLEN=%d]\n", psidlen);
			S46_DBG("[PSID=%d]\n", psid);
			S46_DBG("[OFFSET=%d]\n", offset);
			ret = 1;
		}

		if (fp && fmr) {
			inet_ntop(AF_INET, &prefix4, addrbuf, sizeof(addrbuf));
			inet_ntop(AF_INET6, &prefix6, addrbuf6, sizeof(addrbuf6));
			fprintf(fp, "%s/%d %s/%d %d %d ", addrbuf, prefix4len, addrbuf6, prefix6len, ealen, offset);
			if (mapfp) {
				fprintf(mapfp, "%s %d %s %d %d %d\n", addrbuf, prefix4len, addrbuf6, prefix6len, ealen, offset);
			}
		}
	}
	free(rules);
	if (fp)
		fclose(fp);
	if (mapfp)
		fclose(mapfp);
	if (ret > 0 && fmrs)
		*fmrs = fmrbuf;
	else if (fmrbuf)
		free(fmrbuf);

	return ret;
}

static char *get_s46_ra_prefix(char *rules, char *addr, size_t addrsz)
{
	char *xrules, *rule, *next_rule, *item, *next;
	char tmp[INET6_ADDRSTRLEN + 1];
	char *s = "::/0";
	int  size;

	if (!rules || *rules == '\0' || (xrules = strdup(rules)) == NULL)
		return NULL;

	for (rule = strtok_r(xrules, " ", &next_rule); rule; rule = strtok_r(NULL, " ", &next_rule)) {
		for (item = strtok_r(rule, ",", &next); item; item = strtok_r(NULL, ",", &next)) {
			if (!strstr(item, s)) /* skip ::/0 */
				if (sscanf(item, "%[^/]/%d", tmp, &size) == 2) {
					if (f_exists(S46_DEBUG))
						S46_DBG("prefx:[%s/%d]\n", tmp, size);
					snprintf(addr, addrsz, "%s/%d", tmp, size);
					free(xrules);
					return addr;
				} else
					continue;
			else {
				continue;
			}
		}
	}
	if (xrules)
		free(xrules);

	return NULL;
}

static char *get_s46_ra_routes(char *rules, char *addr, size_t addrsz)
{
	char *xrules, *rule, *next_rule;

	if (!rules || *rules == '\0' || (xrules = strdup(rules)) == NULL)
		return NULL;

	for (rule = strtok_r(xrules, " ", &next_rule); rule; rule = strtok_r(NULL, " ", &next_rule)) {
		/* only get first route at the moment */
		if (sscanf(rule, "%*[^,],%[^,]", addr) == 1 ) {
			S46_DBG("ra_routes:[%s]\n", addr);
			free(xrules);
			return addr;
		}
	}
	if (xrules)
		free(xrules);

	return NULL;
}
#endif

static int
bound6(char *wan_ifname, int bound)
{
	char *lan_ifname = nvram_safe_get("lan_ifname");
	struct in6_addr range;
	char addr[INET6_ADDRSTRLEN + 1], *value;
	char tmp[100], *next;
	int wanaddr_changed, prefix_changed, dns_changed;
	int size, start, end, intval;
	char prefix[sizeof("wanXXXXXXXXXX_")];
	int wan_unit;

	if (wan_ifname && !strncmp(wan_ifname, "ppp", 3))
		wan_unit = ppp_ifunit(wan_ifname);
	else
		wan_unit = wan_ifunit(wan_ifname);
	snprintf(prefix, sizeof(prefix), "wan%d_", wan_unit);

	snprintf(tmp, sizeof(tmp), "/tmp/%sbound6.env", prefix);
	envsave(tmp);

#ifdef RTCONFIG_SOFTWIRE46
	int wan_proto = -1;
	int ra_changed = 0;
	char ra_prefix[INET6_ADDRSTRLEN + sizeof("/128")];
	char ra_route[INET6_ADDRSTRLEN];

	switch (wan_proto = get_wan_proto(prefix)) {
		int i;
	case WAN_MAPE:
	case WAN_DSLITE:
	case WAN_V6PLUS:
	case WAN_OCNVC:
		i = 0;
		S46_DBG("[wan_if]:[%s], [bound]:[%d]\n", wan_ifname, bound);
		while(environ[i] != NULL) {
			S46_DBG("ENV:[%s]\n", environ[i]);
			i++;
		}
		break;
	}

	value = safe_getenv("RA_ADDRESSES");
	if (*value) {
		foreach(tmp, value, next) {
			char *ptr = tmp;
			value = strsep(&ptr,",");
			break; /* only first prefix at the moment */
		}
		if (sscanf(value, "%[^/]/%d", addr, &size) == 2) {
			nvram_set(ipv6_nvname_by_unit("ipv6_ra_addr", wan_unit), addr);
			nvram_set_int(ipv6_nvname_by_unit("ipv6_ra_length", wan_unit), size);
			S46_DBG("[RA_ADDR=%s/%d]\n", addr, size);
		}
	}

	value = safe_getenv("RA_ROUTES");
	if (*value) {
		if (get_s46_ra_routes(value, ra_route, sizeof(ra_route)))
			nvram_set(ipv6_nvname_by_unit("ipv6_ra_route", wan_unit), ra_route);
		if (get_s46_ra_prefix(value, ra_prefix, sizeof(ra_prefix))) {
			if (sscanf(ra_prefix, "%[^/]/%d", addr, &size) == 2) {
				ra_changed = (!nvram_match(ipv6_nvname_by_unit("ipv6_ra_prefix", wan_unit), addr) ||
					       nvram_get_int(ipv6_nvname_by_unit("ipv6_ra_perfix_length", wan_unit)) != size);
				nvram_set(ipv6_nvname_by_unit("ipv6_ra_prefix", wan_unit), addr);
				nvram_set_int(ipv6_nvname_by_unit("ipv6_ra_prefix_length", wan_unit), size);
			}
		}
	}

	value = safe_getenv("AFTR");
	if (*value) {
		foreach(tmp, value, next) {
			nvram_set(ipv6_nvname_by_unit("ipv6_s46_env_aftr", wan_unit), tmp);
			S46_DBG("[AFTR=%s]\n", tmp);
			break; /* only first AFTR at the moment */
		}
	}
#endif

	value = safe_getenv("RA_HOPLIMIT");
	if (*value && (intval = atoi(value)))
		ipv6_sysconf(wan_ifname, "hop_limit", intval);

	value = safe_getenv("RA_MTU");
	if (*value && (intval = atoi(value)) && intval < ifconfig_mtu(wan_ifname, 0)) {
		ipv6_sysconf(wan_ifname, "mtu", intval);
		ipv6_sysconf(lan_ifname, "mtu", intval);
	} else if ((intval = ipv6_getconf(wan_ifname, "mtu")))
		ipv6_sysconf(lan_ifname, "mtu", intval);

	value = safe_getenv("ADDRESSES");
	if (*value) {
		foreach(tmp, value, next) {
			char *ptr = tmp;
			value = strsep(&ptr,",");
			break; /* only first address at the moment */
		}
	}
	wanaddr_changed = !nvram_match(ipv6_nvname_by_unit("ipv6_wan_addr", wan_unit), value);
	if (wanaddr_changed) {
		if (nvram_invmatch(ipv6_nvname_by_unit("ipv6_wan_addr", wan_unit), ""))
			eval("ip", "-6", "addr", "del", nvram_safe_get(ipv6_nvname_by_unit("ipv6_wan_addr", wan_unit)), "dev", wan_ifname);
		nvram_set(ipv6_nvname_by_unit("ipv6_wan_addr", wan_unit), value);
	}
	if (*value)
		eval("ip", "-6", "addr", "add", value, "dev", wan_ifname);

	prefix_changed = 0;
	if (get_ipv6_service_by_unit(wan_unit) == IPV6_NATIVE_DHCP &&
	    nvram_get_int(ipv6_nvname_by_unit("ipv6_dhcp_pd", wan_unit))) {
		value = safe_getenv("PREFIXES");
		if (*value) {
			foreach(tmp, value, next) {
				char *ptr = tmp;
				value = strsep(&ptr,",");
				break; /* only first prefix at the moment */
			}
		}
		if (sscanf(value, "%[^/]/%d", addr, &size) != 2)
			goto skip;

		prefix_changed = (!nvram_match(ipv6_nvname_by_unit("ipv6_prefix", wan_unit), addr) ||
					nvram_get_int(ipv6_nvname_by_unit("ipv6_prefix_length", wan_unit)) != size);
		if (prefix_changed) {
			eval("ip", "-6", "addr", "flush", "scope", "global", "dev", lan_ifname);
			nvram_set(ipv6_nvname_by_unit("ipv6_rtr_addr", wan_unit), "");
			nvram_set(ipv6_nvname_by_unit("ipv6_prefix", wan_unit), addr);
			nvram_set_int(ipv6_nvname_by_unit("ipv6_prefix_length", wan_unit), size);
			logmessage("dhcp6 client", "WAN Prefix Size Requested:/%d, Received:/%d",
				 nvram_get_int(ipv6_nvname("ipv6_prefix_len_wan")), size);
			nvram_set_int(ipv6_nvname("ipv6_prefix_len_wan"), size);
		}
		if (*addr)
			add_ip6_lanaddr();

		if (prefix_changed && nvram_get_int(ipv6_nvname_by_unit("ipv6_autoconf_type", wan_unit))) {
			/* TODO: rework WEB UI to specify ranges without prefix
			 * TODO: add size checking, now range takes all of 16 bit */
			start = (inet_pton(AF_INET6, nvram_safe_get(ipv6_nvname_by_unit("ipv6_dhcp_start", wan_unit)), &range) > 0) ?
			    ntohs(range.s6_addr16[7]) : 0x1000;
			end = (inet_pton(AF_INET6, nvram_safe_get(ipv6_nvname_by_unit("ipv6_dhcp_end", wan_unit)), &range) > 0) ?
			    ntohs(range.s6_addr16[7]) : 0x2000;

			value = nvram_safe_get(ipv6_nvname_by_unit("ipv6_prefix", wan_unit));
			inet_pton(AF_INET6, *value ? value : "::", &range);

			range.s6_addr16[7] = (start < end) ? htons(start) : htons(end);
			inet_ntop(AF_INET6, &range, addr, sizeof(addr));
			nvram_set(ipv6_nvname_by_unit("ipv6_dhcp_start", wan_unit), addr);
			range.s6_addr16[7] = (start < end) ? htons(end) : htons(start);
			inet_ntop(AF_INET6, &range, addr, sizeof(addr));
			nvram_set(ipv6_nvname_by_unit("ipv6_dhcp_end", wan_unit), addr);
		}
	}
skip:
	if (*safe_getenv("RDNSS")) {
		dns_changed = nvram_set_env(ipv6_nvname_by_unit("ipv6_get_dns", wan_unit), "RDNSS");
		dns_changed += nvram_set_env(ipv6_nvname_by_unit("ipv6_get_domain", wan_unit), "DOMAINS");
	} else {
		dns_changed = nvram_set_env(ipv6_nvname_by_unit("ipv6_get_dns", wan_unit), "RA_DNS");
		dns_changed += nvram_set_env(ipv6_nvname_by_unit("ipv6_get_domain", wan_unit), "RA_DOMAINS");
	}
	if (dns_changed && nvram_get_int(ipv6_nvname_by_unit("ipv6_dnsenable", wan_unit)))
		update_resolvconf();

	if (bound == 1 || wanaddr_changed || prefix_changed) {
		char *address = nvram_safe_get(ipv6_nvname_by_unit("ipv6_wan_addr", wan_unit));
		char *prefix = nvram_safe_get(ipv6_nvname_by_unit("ipv6_prefix", wan_unit));

		if (*prefix) {
			snprintf(addr, sizeof(addr), "%s/%d", prefix, nvram_get_int(ipv6_nvname_by_unit("ipv6_prefix_length", wan_unit)));
			prefix = addr;
		}
		logmessage("dhcp6 client", "%s %s%s%s%s%s",
			bound ? "bound" : "informed",
			*address ? "address " : "", address, *address ? ", " : "",
			*prefix ? "prefix " : "", prefix);
#ifdef RTCONFIG_IGD2
		notify_rc("restart_upnp");
#endif

#ifdef RTCONFIG_WIREGUARD
		update_wgs_client_ep();
#endif
	}

#ifdef RTCONFIG_SOFTWIRE46
	switch (wan_proto = get_wan_proto(prefix)) {
		char peerbuf[INET6_ADDRSTRLEN];
		char addr6buf[INET6_ADDRSTRLEN];
		char addr4buf[INET_ADDRSTRLEN];
		char *rules, *fmrs, *rulebuf;
		int s46_changed, offset, psidlen, psid, draft;

		struct in6_addr lanaddr;
		char addr6[INET6_ADDRSTRLEN];
		char ipbuf[INET6_ADDRSTRLEN + 4];

	case WAN_LW4O6:
		if (!nvram_get_int(strcat_r(prefix, "dhcpenable_x", tmp)))
			break;
		rules = safe_getenv("LW4O6");
		rulebuf = NULL;
		draft = 0;
		goto s46_mapcalc;
	case WAN_MAPE:
		if (!nvram_get_int(strcat_r(prefix, "dhcpenable_x", tmp)))
			break;
		rules = safe_getenv("MAPE");
		rulebuf = NULL;
		draft = 0;
		goto s46_mapcalc;
	case WAN_V6PLUS:
	case WAN_OCNVC:
		draft = 1;
	case WAN_DSLITE:
		/* Workaround for system boot case. */
		if (!get_s46_ra(wan_unit))
			S46_DBG("[Warning] Unable to obtain RA.\n");

		if (get_ipv6_service_by_unit(wan_unit) == IPV6_NATIVE_DHCP) {
			if (ra_changed)
				eval("ip", "-6", "addr", "flush", "scope", "global", "dev", lan_ifname);
			if (nvram_match(ipv6_nvname_by_unit("ipv6_ra_prefix", wan_unit), "")) {
				inet_pton(AF_INET6, addr, &lanaddr);
				lanaddr.s6_addr16[7] = htons(0x0001);
				inet_ntop(AF_INET6, &lanaddr, addr6, sizeof(addr6));
				if (*addr6) {
					snprintf(ipbuf, sizeof(ipbuf), "%s/%d", addr6, nvram_get_int(ipv6_nvname_by_unit("ipv6_ra_prefix_length", wan_unit)) ? : 64);
					eval("ip", "-6", "addr", "add", ipbuf, "dev", nvram_safe_get("lan_ifname"));
					S46_DBG("[CMD]:[ip -6 addr add %s dev %s]\n", ipbuf, nvram_safe_get("lan_ifname"));
				}
			}
		}

		//FIXME: wait for DNS
		sleep(5);

		if (wan_proto == WAN_V6PLUS) {
			if (check_v6plusd(wan_unit))
				S46_DBG("[START] v6plusd\n");
			else {
				if (bound == 1 || wanaddr_changed || prefix_changed) {
					snprintf(tmp, sizeof(tmp), V6PLUSD_PIDFILE, wan_unit);
					kill_pidfile_s(tmp, SIGUSR1);
				}
			}
		} else if (wan_proto == WAN_OCNVC) {
			if (check_ocnvcd(wan_unit))
				S46_DBG("[START] ocnvcd\n");
			else {
				if (bound == 1 || wanaddr_changed || prefix_changed) {
					snprintf(tmp, sizeof(tmp), OCNVCD_PIDFILE, wan_unit);
					kill_pidfile_s(tmp, SIGUSR1);
				}
			}
		} else {
			if (check_dslited(wan_unit))
				S46_DBG("[START] dslited\n");
			else {
				if (bound == 1 || wanaddr_changed || prefix_changed) {
					snprintf(tmp, sizeof(tmp), DSLITED_PIDFILE, wan_unit);
					kill_pidfile_s(tmp, SIGUSR1);
				}
			}
		}
		break;
	s46_mapcalc:
		if (s46_mapcalc(wan_unit, wan_proto, rules, peerbuf, sizeof(peerbuf), addr6buf, sizeof(addr6buf),
				addr4buf, sizeof(addr4buf), &offset, &psidlen, &psid, &fmrs, draft) <= 0) {
			peerbuf[0] = addr6buf[0] = addr4buf[0] = '\0';
			offset = 0, psidlen = 0, psid = 0;
			fmrs = NULL;
		}
		free(rulebuf);

		if (!fmrs) {
			S46_DBG("[Err] FMR is NULL.\n");
			break;
		}

		// Delete the last blank character
		fmrs[strlen(fmrs)-1] = '\0';

		s46_changed = nvram_set_check(ipv6_nvname_by_unit("ipv6_s46_peer", wan_unit), peerbuf);
		s46_changed += nvram_set_check(ipv6_nvname_by_unit("ipv6_s46_addr6", wan_unit), addr6buf);
		s46_changed += nvram_set_check(ipv6_nvname_by_unit("ipv6_s46_addr4", wan_unit), addr4buf);
		s46_changed += nvram_set_check(ipv6_nvname_by_unit("ipv6_s46_fmrs", wan_unit), fmrs ? : "");
		free(fmrs);
		if (bound == 1 || s46_changed) {
			nvram_set_int(ipv6_nvname_by_unit("ipv6_s46_offset", wan_unit), offset);
			nvram_set_int(ipv6_nvname_by_unit("ipv6_s46_psidlen", wan_unit), psidlen);
			nvram_set_int(ipv6_nvname_by_unit("ipv6_s46_psid", wan_unit), psid);
			stop_s46_tunnel(wan_unit, 0);
			start_s46_tunnel(wan_unit);
		}

		break;
	default:
		if (nvram_match("x_Setting", "0") && !strncmp(nvram_safe_get("territory_code"), "JP", 2)) {
			get_s46_ra(wan_unit);
		}
	}
#endif

#ifdef RTCONFIG_NTPD
	if (!nvram_get_int("ntp_ready")) {
#endif
		refresh_ntpc();
	}

#ifdef RTCONFIG_INADYN
	if (nvram_get_int("ddns_enable_x") && nvram_get_int("ddns_ipv6_update")) {
		stop_ddns();
		start_ddns(NULL);
	}
#endif
        if (nvram_get_int("telnetd_enable"))
                notify_rc("restart_telnetd");

	return 0;
}

static int
ra_updated6(char *wan_ifname)
{
	char *lan_ifname = nvram_safe_get("lan_ifname");
	char *value;
	int dns_changed, intval;
	int wan_unit;
	char tmp[64];

	if (wan_ifname && !strncmp(wan_ifname, "ppp", 3))
		wan_unit = ppp_ifunit(wan_ifname);
	else
		wan_unit = wan_ifunit(wan_ifname);
	snprintf(tmp, sizeof(tmp), "/tmp/wan%d_ra.env", wan_unit);
	envsave(tmp);

	value = safe_getenv("RA_HOPLIMIT");
	if (*value && (intval = atoi(value)))
		ipv6_sysconf(wan_ifname, "hop_limit", intval);

	value = safe_getenv("RA_MTU");
	if (*value && (intval = atoi(value)) && intval < ifconfig_mtu(wan_ifname, 0)) {
		ipv6_sysconf(wan_ifname, "mtu", intval);
		ipv6_sysconf(lan_ifname, "mtu", intval);
	}

	if (*safe_getenv("RDNSS")) {
		dns_changed = nvram_set_env(ipv6_nvname_by_unit("ipv6_get_dns", wan_unit), "RDNSS");
		dns_changed += nvram_set_env(ipv6_nvname_by_unit("ipv6_get_domain", wan_unit), "DOMAINS");
	} else {
		dns_changed = nvram_set_env(ipv6_nvname_by_unit("ipv6_get_dns", wan_unit), "RA_DNS");
		dns_changed += nvram_set_env(ipv6_nvname_by_unit("ipv6_get_domain", wan_unit), "RA_DOMAINS");
	}
	if (dns_changed && nvram_get_int(ipv6_nvname_by_unit("ipv6_dnsenable", wan_unit)))
		update_resolvconf();

	return 0;
}

int dhcp6c_wan(int argc, char **argv)
{

	if (argv[2]) run_custom_script("dhcpc-event", 0, argv[2], "6");

	if (!argv[1] || !argv[2])
		return EINVAL;
	else if (strcmp(argv[2], "started") == 0)
		return deconfig6(argv[1], 0);
	else if ( strcmp(argv[2], "stopped") == 0)
		return deconfig6(argv[1], 1);
	else if ( strcmp(argv[2], "unbound") == 0)
		return deconfig6(argv[1], 2);		
	else if (strcmp(argv[2], "bound") == 0)
		return bound6(argv[1], 1);
	else if (strcmp(argv[2], "updated") == 0 ||
		 strcmp(argv[2], "rebound") == 0)
		return bound6(argv[1], 2);
	else if (strcmp(argv[2], "informed") == 0)
		return bound6(argv[1], 0);
	else if (strcmp(argv[2], "ra-updated") == 0)
		return ra_updated6(argv[1]);

	return 0;
}

int
start_dhcp6c(void)
{
	char *wan_ifname = (char *) get_wan6face();
	char mode[8] = "try";
	char *dhcp6c_argv[] = { "odhcp6c",
		"-df",
		"-R",
		"-s", "/tmp/dhcp6c",
		"-N", mode,
		NULL, NULL,	/* -c duid */
		NULL, NULL,	/* -P len:iaidhex */
		NULL, NULL,	/* -rdns -rdomain */
#ifdef RTCONFIG_SOFTWIRE46
		NULL,		/* -rS46_CONT_LW / -rS46_CONT_MAPE */
#endif
		NULL,		/* -v */
		NULL,		/* -k */
		NULL,		/* interface */
		NULL };
	int index = 7;
	struct duid duid;
	char duid_arg[sizeof(duid)*2+1];
	char prefix_arg[sizeof("128:xxxxxxxx")];
	int service, i;
#ifdef RTCONFIG_SOFTWIRE46
	int wan_proto;
#endif

#ifndef RT4GAC68U
	if (getpid() != 1) {
		notify_rc_and_wait_1min("start_dhcp6c");
		return 0;
	}
#endif

	/* Check if enabled */
	service = get_ipv6_service();
	if (
#ifdef RTCONFIG_6RELAYD
	    service != IPV6_PASSTHROUGH &&
#endif
	    service != IPV6_NATIVE_DHCP)
		return 0;

	if (!wan_ifname || *wan_ifname == '\0')
		return -1;

#ifdef RTCONFIG_SOFTWIRE46
	wan_proto = get_ipv4_service();
#endif
	stop_dhcp6c();

	if (get_duid(&duid)) {
		bin2hex(duid_arg, sizeof(duid_arg), &duid, sizeof(duid));
		dhcp6c_argv[index++] = "-c";
		dhcp6c_argv[index++] = duid_arg;
	}

	if (service == IPV6_NATIVE_DHCP &&
	    nvram_get_int(ipv6_nvname("ipv6_dhcp_pd"))) {
		char hwname[sizeof("wanXXXXXXXXXX_hwaddr")];
		unsigned long iaid;

		/* Generate IA_PD IAID from the last 7 digits of WAN MAC */
		snprintf(hwname, sizeof(hwname), "wan%d_hwaddr", wan_primary_ifunit_ipv6());
		iaid = ether_atoe(nvram_safe_get(hwname), duid.ea) ?
			((unsigned long)(duid.ea[3] & 0x0f) << 16) |
			((unsigned long)(duid.ea[4]) << 8) |
			((unsigned long)(duid.ea[5])) : 1;
		i = (nvram_get_int(ipv6_nvname("ipv6_prefix_len_wan")) ? : 0);
		if ((i < 48) || (i > 64))
			i = 0;
		snprintf(prefix_arg, sizeof(prefix_arg), "%d:%lx", i, iaid);
		dhcp6c_argv[index++] = "-P";
		dhcp6c_argv[index++] = prefix_arg;
	}

	if (nvram_get_int(ipv6_nvname("ipv6_dnsenable"))) {
		dhcp6c_argv[index++] = "-r23";	/* dns */
		dhcp6c_argv[index++] = "-r24";	/* domain */
	}

#ifdef RTCONFIG_SOFTWIRE46
	switch (wan_proto) {
	case WAN_LW4O6:
		dhcp6c_argv[index++] = "-r96";	/* S46_CONT_LW */
		break;
	case WAN_MAPE:
	case WAN_V6PLUS:
		dhcp6c_argv[index++] = "-r94";	/* S46_CONT_MAPE */
		break;
	case WAN_OCNVC:
		/* Send Information-Request message */
		snprintf(mode, sizeof(mode), "%s", "none");
		dhcp6c_argv[index++] = "-r94";	/* S46_CONT_MAPE */
		break;
	case WAN_DSLITE:
		/* Send Information-Request message */
		snprintf(mode, sizeof(mode), "%s", "none");
		dhcp6c_argv[index++] = "-r64";	/* OPTION_AFTR_NAME */
		break;
	}
#endif

	if (nvram_get_int("ipv6_debug"))
		dhcp6c_argv[index++] = "-v";

	if (nvram_get_int(ipv6_nvname("ipv6_dhcp6c_release")) == 0)
		dhcp6c_argv[index++] = "-k";

	dhcp6c_argv[index++] = wan_ifname;

	return _eval(dhcp6c_argv, NULL, 0, NULL);
}

void stop_dhcp6c(void)
{
	killall_tk("odhcp6c");
}

#ifdef RTCONFIG_6RELAYD
int start_6relayd(void)
{
	char *lan_ifname = nvram_safe_get("lan_ifname");
	char *wan_ifname = (char *) get_wan6face();
	char *relayd_argv[] = { "6relayd",
		"-drs",
		"-Rrelay", "-Dserver", "-N",
		"-n",
		NULL,		/* -v */
		NULL,		/* master ifname */
		NULL,		/* internal ifname */
		NULL };
	int index = 6;

	if (get_ipv6_service() != IPV6_PASSTHROUGH)
		return 0;

	if (!wan_ifname || *wan_ifname == '\0')
		return -1;

	stop_6relayd();

	if (nvram_get_int("ipv6_debug"))
		relayd_argv[index++] = "-v";

	relayd_argv[index++] = wan_ifname;
	relayd_argv[index++] = lan_ifname;

	return _eval(relayd_argv, NULL, 0, NULL);
}

void stop_6relayd(void)
{
	killall_tk("6relayd");
}
#endif // RTCONFIG_6RELAYD

#endif // RTCONFIG_IPV6
