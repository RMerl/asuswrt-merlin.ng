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
 * Copyright 2004, ASUSTeK Inc.
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND ASUS GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */
#include "rc.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <syslog.h>															
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#ifdef RTCONFIG_MULTIWAN_IF
#include "multi_wan.h"
#endif
#include <rtstate.h>

char *ppp_escape(char *src, char *buf, size_t size)
{
	const char special_chars[] = "'\"\\";
	char *dst = buf;
	char *end = buf + size - 1;

	if (src == NULL || dst == NULL || size == 0)
		return NULL;

	while (*src && dst < end) {
		if (strchr(special_chars, *src) != NULL)
			*dst++ = '\\';
		*dst++ = *src++;
	}
	*dst++ = '\0';

	return buf;
}

char *ppp_safe_escape(char *src, char *buf, size_t size)
{
	return ppp_escape(src, buf, size) ? : "";
}

int
start_pppd(int unit)
{
	FILE *fp;
	char options[80];
	char *pppd_argv[] = { "/usr/sbin/pppd", "file", options, NULL};
	char tmp[100], prefix[sizeof("wanXXXXXXXXXX_")];
	char buf[256];	/* although maximum length of pppoe_username/pppoe_passwd is 64. pppd accepts up to 256 characters. */
	mode_t mask;
	int ret = 0;
#ifdef RTCONFIG_DSL
	char dsl_prefix[16] = {0};
	get_dsl_prefix_by_wan_unit(unit, dsl_prefix, sizeof(dsl_prefix));
#endif

	_dprintf("%s: unit=%d.\n", __FUNCTION__, unit);

#if 0
#ifdef RTCONFIG_DUALWAN
	if (!strstr(nvram_safe_get("wans_dualwan"), "none")
		//&& (!strcmp(nvram_safe_get("wans_mode"), "fo") || !strcmp(nvram_safe_get("wans_mode"), "fb"))
		&& !strcmp(nvram_safe_get("wans_mode"), "fo")
		&& (wan_primary_ifunit() != unit)) {
		_dprintf("%s: skip non-primary unit %d\n", __FUNCTION__, unit);
		return -1;
	}
#endif
#endif

	snprintf(prefix, sizeof(prefix), "wan%d_", unit);
	sprintf(options, "/tmp/ppp/options.wan%d", unit);

#ifdef RTCONFIG_MULTI_PPP
	if (is_mtppp_unit(unit)) {
		snprintf(prefix, sizeof(prefix), "wan%d_", get_mtppp_base_unit(unit));
	}
#endif

	mask = umask(0000);

	/* Generate options file */
	if (!(fp = fopen(options, "w"))) {
		perror(options);
		umask(mask);
		return -1;
	}

	umask(mask);

	/* do not authenticate peer and do not use eap */
	fprintf(fp, "noauth\n");
	fprintf(fp, "refuse-eap\n");
	fprintf(fp, "user '%s'\n",
		ppp_safe_escape(nvram_safe_get(strcat_r(prefix, "pppoe_username", tmp)), buf, sizeof(buf)));
	fprintf(fp, "password '%s'\n",
		ppp_safe_escape(nvram_safe_get(strcat_r(prefix, "pppoe_passwd", tmp)), buf, sizeof(buf)));

	if (nvram_match(strcat_r(prefix, "proto", tmp), "pptp")) {
		fprintf(fp, "plugin pptp.so\n");
		fprintf(fp, "pptp_server '%s'\n",
			nvram_invmatch(strcat_r(prefix, "heartbeat_x", tmp), "") ?
			nvram_safe_get(strcat_r(prefix, "heartbeat_x", tmp)) :
			nvram_safe_get(strcat_r(prefix, "gateway_x", tmp)));
		fprintf(fp, "pptp_ifname %s\n",
			nvram_safe_get(strcat_r(prefix, "ifname", tmp)));
		/* see KB Q189595 -- historyless & mtu */
		fprintf(fp, "nomppe-stateful mtu 1400\n");
		if (nvram_match(strcat_r(prefix, "pptp_options_x", tmp), "-mppc")) {
			fprintf(fp, "nomppe nomppc\n");
		} else
		if (nvram_match(strcat_r(prefix, "pptp_options_x", tmp), "+mppe-40")) {
			fprintf(fp, "require-mppe\n");
			fprintf(fp, "require-mppe-40\n");
		} else
		if (nvram_match(strcat_r(prefix, "pptp_options_x", tmp), "+mppe-56")) {
			fprintf(fp, "nomppe-40\n"
				    "require-mppe\n"
				    "require-mppe-56\n");
		} else
		if (nvram_match(strcat_r(prefix, "pptp_options_x", tmp), "+mppe-128")) {
			fprintf(fp, "nomppe-40\n"
				    "nomppe-56\n"
				    "require-mppe\n"
				    "require-mppe-128\n");
		}
	} else {
		fprintf(fp, "nomppe nomppc\n");
	}

#ifdef RTCONFIG_DSL_HOST
	if (nvram_match("dslx_transmode", "atm") && nvram_pf_match(dsl_prefix, "proto", "pppoa")) {
		fprintf(fp, "plugin pppoatm.so %d.%d\n"
			, nvram_pf_get_int(dsl_prefix, "vpi")
			, nvram_pf_get_int(dsl_prefix, "vci"));
		if (nvram_pf_get_int(dsl_prefix, "encap"))
			fprintf(fp, "vc-encaps\n");
		else
			fprintf(fp, "llc-encaps\n");
	}
	else
#endif
	if (nvram_match(strcat_r(prefix, "proto", tmp), "pppoe")) {
		fprintf(fp, "plugin rp-pppoe.so nic-%s\n",
			nvram_safe_get(strcat_r(prefix, "ifname", tmp)));

		if (nvram_invmatch(strcat_r(prefix, "pppoe_service", tmp), "")) {
			fprintf(fp, "rp_pppoe_service '%s'\n",
				nvram_safe_get(strcat_r(prefix, "pppoe_service", tmp)));
		}

		if (nvram_invmatch(strcat_r(prefix, "pppoe_ac", tmp), "")) {
			fprintf(fp, "rp_pppoe_ac '%s'\n",
				nvram_safe_get(strcat_r(prefix, "pppoe_ac", tmp)));
		}

		if (nvram_invmatch(strcat_r(prefix, "pppoe_hostuniq", tmp), "")) {
			fprintf(fp, "host-uniq %s\n",
				nvram_safe_get(strcat_r(prefix, "pppoe_hostuniq", tmp)));
		}
#ifdef RTCONFIG_DSL
#ifdef RTCONFIG_DSL_REMOTE
		if (nvram_match("dslx_transmode", "atm")
			&& nvram_pf_match(dsl_prefix, "proto", "pppoa")
		) {
			char *dsl_mac = NULL;
			FILE *fp_dsl_mac;
			int timeout = 10; /* wait up to 10 seconds */

			while (timeout--) {
				fp_dsl_mac = fopen("/tmp/adsl/tc_mac.txt","r");
				if (fp_dsl_mac != NULL) {
					dsl_mac = fgets(tmp, sizeof(tmp), fp_dsl_mac);
					dsl_mac = strsep(&dsl_mac, "\r\n");
					fclose(fp_dsl_mac);
					break;
				}
				usleep(1000*1000);
			}
			fprintf(fp, "rp_pppoe_sess %d:%s\n", 154,
				(dsl_mac && *dsl_mac) ? dsl_mac : "00:11:22:33:44:55");
		}
#endif
#endif

		fprintf(fp, "mru %d\n", nvram_valid_get_int(strcat_r(prefix, "pppoe_mru", tmp), 128, 1500, 1492));
		fprintf(fp, "mtu %d\n", nvram_valid_get_int(strcat_r(prefix, "pppoe_mtu", tmp), 128, 1500, 1492));
	}

	if (nvram_invmatch(strcat_r(prefix, "proto", tmp), "l2tp")) {
		ret = nvram_get_int(strcat_r(prefix, "pppoe_idletime", tmp));
		if (ret && nvram_get_int(strcat_r(prefix, "pppoe_demand", tmp))) {
			fprintf(fp, "idle %d ", ret);
			if (nvram_invmatch(strcat_r(prefix, "pppoe_txonly_x", tmp), "0"))
				fprintf(fp, "tx_only ");
			fprintf(fp, "demand\n");
		}
		fprintf(fp, "persist\n");
	}

	if (nvram_match(strcat_r(prefix, "pppoe_auth", tmp), "pap")) {
		fprintf(fp, "-chap\n"
					"-mschap\n"
					"-mschap-v2\n"
					);
	}
	else if (nvram_match(strcat_r(prefix, "pppoe_auth", tmp), "chap")) {
		fprintf(fp, "-pap\n");
	}

#ifdef RTCONFIG_SPECIFIC_PPPOE
	if(unit == WAN_UNIT_FIRST && nvram_get_int(strcat_r(prefix, "pppoe_specific", tmp)) == 1)
	{
		nvram_set("freeze_duck", "864000");
		srand(nvram_get_int("secret_code"));

		int holdoff_min = nvram_get_int(strcat_r(prefix, "pppoe_holdoff_min", tmp))?:5;
		int holdoff_max = nvram_get_int(strcat_r(prefix, "pppoe_holdoff_max", tmp))?:60;
		int holdoff_base = holdoff_max-holdoff_min+1;
		int holdoff_rand = rand()%holdoff_base+holdoff_min;

		fprintf(fp, "holdoff %d\n", holdoff_rand);
		fprintf(fp, "maxfail %d\n", nvram_get_int(strcat_r(prefix, "pppoe_maxfail", tmp)));
	}
	else
#endif
	{
		fprintf(fp, "holdoff %d\n", nvram_get_int(strcat_r(prefix, "pppoe_holdoff", tmp)) ? : 10);
		fprintf(fp, "maxfail %d\n", nvram_get_int(strcat_r(prefix, "pppoe_maxfail", tmp)));
	}

#if defined(RTCONFIG_IPV6) && (defined(RTAX82_XD6) || defined(RTAX82_XD6S) || defined(XD6_V2) || defined(ET12))
	if (!strncmp(nvram_safe_get("territory_code"), "CH", 2) &&
		nvram_match(ipv6_nvname("ipv6_only"), "1"))
	switch (get_ipv6_service_by_unit(unit)) {
	case IPV6_NATIVE_DHCP:
	case IPV6_MANUAL:
#ifdef RTCONFIG_6RELAYD
	case IPV6_PASSTHROUGH:
#endif
		if (nvram_match(ipv6_nvname_by_unit("ipv6_ifdev", unit), "ppp")
#ifdef RTCONFIG_DUALWAN
			&& (unit == wan_primary_ifunit_ipv6())
#endif
		) {
			fprintf(fp, "noip\n");
			fprintf(fp, "noipdefault\n");
		}
		break;
	} else
#endif
	{
		if (nvram_get_int(strcat_r(prefix, "dnsenable_x", tmp)))
			fprintf(fp, "usepeerdns\n");

		fprintf(fp, "ipcp-accept-remote ipcp-accept-local noipdefault\n");
	}
	fprintf(fp, "ktune\n");

	/* pppoe set these options automatically */
	/* looks like pptp also likes them */
	fprintf(fp, "default-asyncmap nopcomp noaccomp\n");

	/* pppoe disables "vj bsdcomp deflate" automagically */
	/* ccp should still be enabled - mppe/mppc requires this */
	fprintf(fp, "novj nobsdcomp nodeflate\n");

	/* echo failures */
	if (dualwan_unit__usbif(unit)) {
		fprintf(fp, "lcp-echo-interval %d\n", 6);
		fprintf(fp, "lcp-echo-failure %d\n", 10);
	} else
	if (nvram_get_int(strcat_r(prefix, "ppp_echo", tmp)) == 1) {
		fprintf(fp, "lcp-echo-interval %d\n", nvram_get_int(strcat_r(prefix, "ppp_echo_interval", tmp)));
		fprintf(fp, "lcp-echo-failure %d\n", nvram_get_int(strcat_r(prefix, "ppp_echo_failure", tmp)));
		fprintf(fp, "lcp-echo-adaptive\n");
	}

	fprintf(fp, "unit %d\n", unit);
	fprintf(fp, "linkname wan%d\n", unit);

#ifdef RTCONFIG_IPV6
	switch (get_ipv6_service_by_unit(unit)) {
	case IPV6_NATIVE_DHCP:
	case IPV6_MANUAL:
#ifdef RTCONFIG_6RELAYD
	case IPV6_PASSTHROUGH:
#endif
		if (nvram_match(ipv6_nvname_by_unit("ipv6_ifdev", unit), "ppp")
#ifdef RTCONFIG_DUALWAN
			&& (unit == wan_primary_ifunit_ipv6()
#ifdef RTCONFIG_MULTIWAN_IF
				|| is_mtwan_unit(unit)
#endif
			)
#endif
		)
			fprintf(fp, "+ipv6\n");
		break;
	}
#endif

	/* user specific options */
	fprintf(fp, "%s\n",
		nvram_safe_get(strcat_r(prefix, "pppoe_options_x", tmp)));

	fclose(fp);

	/* shut down previous instance if any */
	stop_pppd(unit);
#if defined(RTCONFIG_SOC_IPQ8074)
	sleep(2);
#endif

	if (nvram_match(strcat_r(prefix, "proto", tmp), "l2tp"))
	{
		if (!(fp = fopen("/tmp/l2tp.conf", "w"))) {
			perror(options);
			return -1;
		}

		fprintf(fp,
			"global\n\n"
			"load-handler \"sync-pppd.so\"\n"
			"load-handler \"cmd.so\"\n\n"
			"section sync-pppd\n\n"
			"lac-pppd-opts \"file %s\"\n\n"
			"section peer\n"
			"port 1701\n"
			"peername %s\n"
			"ifname %s\n"
			"hostname %s\n"
			"lac-handler sync-pppd\n"
			"persist yes\n"
			"maxfail %d\n"
			"holdoff %d\n"
			"hide-avps no\n"
			"section cmd\n\n",
			options,
			nvram_invmatch(strcat_r(prefix, "heartbeat_x", tmp), "") ?
				nvram_safe_get(strcat_r(prefix, "heartbeat_x", tmp)) :
				nvram_safe_get(strcat_r(prefix, "gateway_x", tmp)),
			nvram_safe_get(strcat_r(prefix, "ifname", tmp)),
			nvram_invmatch(strcat_r(prefix, "hostname", tmp), "") ?
				nvram_safe_get(strcat_r(prefix, "hostname", tmp)) : "localhost",
			nvram_get_int(strcat_r(prefix, "pppoe_maxfail", tmp))  ? : 32767,
			nvram_get_int(strcat_r(prefix, "pppoe_holdoff", tmp)) ? : 10);

		fclose(fp);

		/* launch l2tp */
		eval("/usr/sbin/l2tpd");

		ret = 3;
		do {
			_dprintf("%s: wait l2tpd up at %d seconds...\n", __FUNCTION__, ret);
			usleep(1000*1000);
		} while (!pids("l2tpd") && ret--);

		/* start-session */
		ret = eval("/usr/sbin/l2tp-control", "start-session 0.0.0.0");

		/* pppd sync nodetach noaccomp nobsdcomp nodeflate */
		/* nopcomp novj novjccomp file /tmp/ppp/options.l2tp */

	} else
		ret = _eval(pppd_argv, NULL, 0, NULL);

	return ret;
}

void
stop_pppd(int unit)
{
	char pidfile[sizeof("/var/run/ppp-wanXXXXXXXXXX.pid")];
	char tmp[100], prefix[sizeof("wanXXXXXXXXXX_")];

	snprintf(pidfile, sizeof(pidfile), "/var/run/ppp-wan%d.pid", unit);
	if (kill_pidfile_s(pidfile, SIGHUP) == 0 &&
	    kill_pidfile_s(pidfile, SIGTERM) == 0) {
		usleep(1000*1000);
		kill_pidfile_tk(pidfile);
	}

	snprintf(prefix, sizeof(prefix), "wan%d_", unit);
	nvram_set(strcat_r(prefix, "pppoe_ifname", tmp), "");
}

int
start_demand_ppp(int unit, int wait)
{
	char tmp[100], prefix[sizeof("wanXXXXXXXXXX_")];
	char *ping_argv[] = { "ping", "-c1", "203.69.138.19", NULL };
	char *value;
	pid_t pid;

	snprintf(prefix, sizeof(prefix), "wan%d_", unit);

	value = nvram_safe_get(strcat_r(prefix, "proto", tmp));
	if (strcmp(value, "pppoe") != 0 && strcmp(value, "pptp") != 0)
		return -1;
	if (nvram_get_int(strcat_r(prefix, "pppoe_demand", tmp)) != 2)
		return -1;

	value = nvram_safe_get(strcat_r(prefix, "gateway", tmp));
	if (inet_addr_(value) != INADDR_ANY)
		ping_argv[2] = value;

	_dprintf("%s: trigger the PPP connection via %s\n", __FUNCTION__, value);
	logmessage("WAN Connection", "trigger the PPP connection via %s", value);

	return _eval(ping_argv, NULL, 0, wait ? NULL : &pid);
}

int
start_pppoe_relay(char *wan_if)
{
	char *pppoerelay_argv[] = {"/usr/sbin/pppoe-relay",
		"-C", "br0",
		"-S", wan_if,
		nvram_match("hide_relayid","1")?"-H":"",
		"-F", NULL};
	pid_t pid;
	int ret = 0;

	killall_tk("pppoe-relay");
	if (nvram_get_int("fw_pt_pppoerelay"))
		ret = _eval(pppoerelay_argv, NULL, 0, &pid);

	return ret;
}

void
stop_pppoe_relay(void)
{
	killall_tk("pppoe-relay");
}
