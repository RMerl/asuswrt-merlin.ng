/**
 * Copyright (C) 2012-2014 Steven Barth <steven@midlink.org>
 * Copyright (C) 2017-2018 Hans Dedecker <dedeckeh@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License v2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <time.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <limits.h>
#include <resolv.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include <string.h>
#include <strings.h>
#include <stdbool.h>

#include <net/if.h>
#include <sys/syscall.h>
#include <arpa/inet.h>

#include <linux/rtnetlink.h>

#ifndef IFA_F_DADFAILED
#define IFA_F_DADFAILED 0x08
#endif

#include "odhcp6c.h"
#include "ra.h"

#ifndef IN6_IS_ADDR_UNIQUELOCAL
#define IN6_IS_ADDR_UNIQUELOCAL(a) \
	((((__const uint32_t *) (a))[0] & htonl (0xfe000000)) \
	 == htonl (0xfc000000))
#endif
#define ARRAY_SEP " ,\t"

static void sighandler(int signal);
static int usage(void);
static int add_opt(const uint16_t code, const uint8_t *data,
		const uint16_t len);
static int parse_opt_data(const char *data, uint8_t **dst,
		const unsigned int type, const bool array);
static int parse_opt(const char *opt);

static uint8_t *state_data[_STATE_MAX] = {NULL};
static size_t state_len[_STATE_MAX] = {0};

static volatile bool signal_io = false;
static volatile bool signal_usr1 = false;
static volatile bool signal_usr2 = false;
static volatile bool signal_term = false;

static int urandom_fd = -1, allow_slaac_only = 0;
static bool bound = false, release = true, ra = false;
static time_t last_update = 0;
static char *ifname = NULL;

static unsigned int script_sync_delay = 10;
static unsigned int script_accu_delay = 1;

static struct odhcp6c_opt opts[] = {
	{ .code = DHCPV6_OPT_CLIENTID, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6_OPT_SERVERID, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6_OPT_IA_NA, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str= NULL },
	{ .code = DHCPV6_OPT_IA_TA, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6_OPT_IA_ADDR, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6_OPT_ORO, .flags = OPT_INTERNAL, .str = NULL },
	{ .code = DHCPV6_OPT_PREF, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6_OPT_ELAPSED, .flags = OPT_INTERNAL, .str = NULL },
	{ .code = DHCPV6_OPT_RELAY_MSG, .flags = OPT_INTERNAL, .str = NULL },
	{ .code = DHCPV6_OPT_AUTH, .flags = OPT_U8 | OPT_NO_PASSTHRU, .str = "authentication" },
	{ .code = DHCPV6_OPT_UNICAST, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6_OPT_STATUS, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6_OPT_RAPID_COMMIT, .flags = OPT_INTERNAL, .str = NULL },
	{ .code = DHCPV6_OPT_USER_CLASS, .flags = OPT_USER_CLASS | OPT_ARRAY, .str = "userclass" },
	{ .code = DHCPV6_OPT_VENDOR_CLASS, .flags = OPT_U8, .str = "vendorclass" },
	{ .code = DHCPV6_OPT_INTERFACE_ID, .flags = OPT_INTERNAL, .str = NULL },
	{ .code = DHCPV6_OPT_RECONF_MESSAGE, .flags = OPT_INTERNAL, .str = NULL },
	{ .code = DHCPV6_OPT_RECONF_ACCEPT, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6_OPT_SIP_SERVER_D, .flags = OPT_DNS_STR | OPT_ORO, .str = "sipserver_d" },
	{ .code = DHCPV6_OPT_SIP_SERVER_A, .flags = OPT_IP6 | OPT_ARRAY | OPT_ORO, .str = "sipserver_a" },
	{ .code = DHCPV6_OPT_DNS_SERVERS, .flags = OPT_IP6 | OPT_ARRAY | OPT_ORO, .str = "dns" },
	{ .code = DHCPV6_OPT_DNS_DOMAIN, .flags = OPT_DNS_STR | OPT_ORO, .str = "search" },
	{ .code = DHCPV6_OPT_IA_PD, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6_OPT_IA_PREFIX, .flags = OPT_INTERNAL, .str = NULL },
	{ .code = DHCPV6_OPT_SNTP_SERVERS, .flags = OPT_IP6 | OPT_ARRAY | OPT_ORO, .str = "sntpservers" },
	{ .code = DHCPV6_OPT_INFO_REFRESH, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU | OPT_ORO | OPT_ORO_STATELESS, .str = NULL },
	{ .code = DHCPV6_OPT_REMOTE_ID, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6_OPT_SUBSCRIBER_ID, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6_OPT_FQDN, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU | OPT_ORO, .str = NULL },
	{ .code = DHCPV6_OPT_ERO, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6_OPT_LQ_QUERY, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6_OPT_CLIENT_DATA, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6_OPT_CLT_TIME, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6_OPT_LQ_RELAY_DATA, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6_OPT_LQ_CLIENT_LINK, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6_OPT_RELAY_ID, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6_OPT_NTP_SERVER, .flags = OPT_U8 | OPT_ORO, .str = "ntpserver" },
	{ .code = DHCPV6_OPT_CLIENT_ARCH_TYPE, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6_OPT_AFTR_NAME, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU | OPT_ORO, .str = NULL },
	{ .code = DHCPV6_OPT_RSOO, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6_OPT_PD_EXCLUDE, .flags = OPT_INTERNAL | OPT_ORO | OPT_ORO_STATEFUL, .str = NULL },
	{ .code = DHCPV6_OPT_VSS, .flags = OPT_U8, .str = "vss" },
	{ .code = DHCPV6_OPT_LINK_LAYER_ADDRESS, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6_OPT_LINK_ADDRESS, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6_OPT_RADIUS, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6_OPT_SOL_MAX_RT, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU | OPT_ORO | OPT_ORO_STATEFUL, .str = NULL },
	{ .code = DHCPV6_OPT_INF_MAX_RT, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU | OPT_ORO | OPT_ORO_STATELESS, .str = NULL },
#ifdef EXT_CER_ID
	{ .code = DHCPV6_OPT_CER_ID, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
#endif
	{ .code = DHCPV6_OPT_DHCPV4_MSG, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6_OPT_S46_RULE, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6_OPT_S46_BR, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6_OPT_S46_DMR, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6_OPT_S46_V4V6BIND, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6_OPT_S46_PORTPARAMS, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6_OPT_S46_CONT_MAPE, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU | OPT_ORO, .str = NULL },
	{ .code = DHCPV6_OPT_S46_CONT_MAPT, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU | OPT_ORO, .str = NULL },
	{ .code = DHCPV6_OPT_S46_CONT_LW, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU | OPT_ORO, .str = NULL },
	{ .code = DHCPV6_OPT_LQ_BASE_TIME, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6_OPT_LQ_START_TIME, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6_OPT_LQ_END_TIME, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6_OPT_ANI_ATT, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6_OPT_ANI_NETWORK_NAME, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6_OPT_ANI_AP_NAME, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6_OPT_ANI_AP_BSSID, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6_OPT_ANI_OPERATOR_ID, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6_OPT_ANI_OPERATOR_REALM, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6_OPT_MUD_URL_V6, .flags = OPT_STR | OPT_NO_PASSTHRU, .str = "mud_url_v6" },
	{ .code = DHCPV6_OPT_F_BINDING_STATUS, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6_OPT_F_CONNECT_FLAGS, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6_OPT_F_DNS_REMOVAL_INFO, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6_OPT_F_DNS_HOST_NAME, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6_OPT_F_DNS_ZONE_NAME, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6_OPT_F_DNS_FLAGS, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6_OPT_F_EXPIRATION_TIME, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6_OPT_F_MAX_UNACKED_BNDUPD, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6_OPT_F_MCLT, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6_OPT_F_PARTNER_LIFETIME, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6_OPT_F_PARTNER_LIFETIME_SENT, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6_OPT_F_PARTNER_DOWN_TIME, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6_OPT_F_PARTNER_RAW_CLT_TIME, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6_OPT_F_PROTOCOL_VERSION, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6_OPT_F_KEEPALIVE_TIME, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6_OPT_F_RECONFIGURE_DATA, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6_OPT_F_RELATIONSHIP_NAME, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6_OPT_F_SERVER_FLAGS, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6_OPT_F_SERVER_STATE, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6_OPT_F_START_TIME_OF_STATE, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6_OPT_F_STATE_EXPIRATION_TIME, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = DHCPV6_OPT_RELAY_PORT, .flags = OPT_INTERNAL | OPT_NO_PASSTHRU, .str = NULL },
	{ .code = 0, .flags = 0, .str = NULL },
};

int main(_unused int argc, char* const argv[])
{
	static struct in6_addr ifid = IN6ADDR_ANY_INIT;
	// Allocate resources
	const char *pidfile = NULL;
	const char *script = "/usr/sbin/odhcp6c-update";
	ssize_t l;
	uint8_t buf[134], *o_data;
	char *optpos;
	uint16_t opttype;
	enum odhcp6c_ia_mode ia_na_mode = IA_MODE_TRY;
	enum odhcp6c_ia_mode ia_pd_mode = IA_MODE_NONE;
	struct odhcp6c_opt *opt;
	int ia_pd_iaid_index = 0;
	int sol_timeout = DHCPV6_SOL_MAX_RT;
	int verbosity = 0;
	bool help = false, daemonize = false;
	int logopt = LOG_PID;
	int c, res;
	unsigned int client_options = DHCPV6_CLIENT_FQDN | DHCPV6_ACCEPT_RECONFIGURE;
	unsigned int ra_options = RA_RDNSS_DEFAULT_LIFETIME;
	unsigned int ra_holdoff_interval = RA_MIN_ADV_INTERVAL;

	while ((c = getopt(argc, argv, "S::N:V:P:FB:c:i:r:Ru:Ux:s:kt:m:Lhedp:fav")) != -1) {
		switch (c) {
		case 'S':
			allow_slaac_only = (optarg) ? atoi(optarg) : -1;
			break;

		case 'N':
			if (!strcmp(optarg, "force")) {
				ia_na_mode = IA_MODE_FORCE;
				allow_slaac_only = -1;
			} else if (!strcmp(optarg, "none"))
				ia_na_mode = IA_MODE_NONE;
			else if (!strcmp(optarg, "try"))
				ia_na_mode = IA_MODE_TRY;
			else
				help = true;
			break;

		case 'V':
			opt = odhcp6c_find_opt(DHCPV6_OPT_VENDOR_CLASS);
			if (!opt) {
				syslog(LOG_ERR, "Failed to set vendor-class option");
				return 1;
			}

			o_data = NULL;
			res = parse_opt_data(optarg, &o_data, opt->flags & OPT_MASK_SIZE,
						(opt->flags & OPT_ARRAY) == OPT_ARRAY);
			if (res > 0) {
				res = add_opt(opt->code, o_data, res);
				if (res) {
					if (res > 0)
						return 1;

					help = true;
				}
			} else
				help = true;

			free(o_data);
			break;

		case 'P':
			if (ia_pd_mode == IA_MODE_NONE)
				ia_pd_mode = IA_MODE_TRY;

			if (allow_slaac_only >= 0 && allow_slaac_only < 10)
				allow_slaac_only = 10;

			char *iaid_begin;
			int iaid_len = 0;
			int prefix_length = strtoul(optarg, &iaid_begin, 10);

			if (*iaid_begin != '\0' && *iaid_begin != ',' && *iaid_begin != ':') {
				syslog(LOG_ERR, "invalid argument: '%s'", optarg);
				return 1;
			}

			struct odhcp6c_request_prefix prefix = { 0, prefix_length };

			if (*iaid_begin == ',' && (iaid_len = strlen(iaid_begin)) > 1)
				memcpy(&prefix.iaid, iaid_begin + 1, iaid_len > 4 ? 4 : iaid_len);
			else if (*iaid_begin == ':')
				prefix.iaid = htonl((uint32_t)strtoul(&iaid_begin[1], NULL, 16));
			else
				prefix.iaid = htonl(++ia_pd_iaid_index);

			if (odhcp6c_add_state(STATE_IA_PD_INIT, &prefix, sizeof(prefix))) {
				syslog(LOG_ERR, "Failed to set request IPv6-Prefix");
				return 1;
			}
			break;

		case 'F':
			allow_slaac_only = -1;
			ia_pd_mode = IA_MODE_FORCE;
			break;

		case 'c':
			l = script_unhexlify(&buf[4], sizeof(buf) - 4, optarg);
			if (l > 0) {
				buf[0] = 0;
				buf[1] = DHCPV6_OPT_CLIENTID;
				buf[2] = 0;
				buf[3] = l;
				if (odhcp6c_add_state(STATE_CLIENT_ID, buf, l + 4)) {
					syslog(LOG_ERR, "Failed to override client-ID");
					return 1;
				}
			} else
				help = true;
			break;

		case 'i':
			if (inet_pton(AF_INET6, optarg, &ifid) != 1)
				help = true;
			break;

		case 'r':
			optpos = optarg;
			while (optpos[0]) {
				opttype = htons(strtoul(optarg, &optpos, 10));
				if (optpos == optarg)
					break;
				else if (optpos[0])
					optarg = &optpos[1];

				if (odhcp6c_add_state(STATE_ORO, &opttype, 2)) {
					syslog(LOG_ERR, "Failed to add requested option");
					return 1;
				}
			}
			break;

		case 'R':
			client_options |= DHCPV6_STRICT_OPTIONS;
			break;

		case 'u':
			opt = odhcp6c_find_opt(DHCPV6_OPT_USER_CLASS);
			if (!opt) {
				syslog(LOG_ERR, "Failed to set user-class option");
				return 1;
			}

			o_data = NULL;
			res = parse_opt_data(optarg, &o_data, opt->flags & OPT_MASK_SIZE,
						(opt->flags & OPT_ARRAY) == OPT_ARRAY);
			if (res > 0) {
				res = add_opt(opt->code, o_data, res);
				if (res) {
					if (res > 0)
						return 1;

					help = true;
				}
			} else
				help = true;

			free(o_data);
			break;

		case 'U':
			client_options |= DHCPV6_IGNORE_OPT_UNICAST;
			break;

		case 's':
			script = optarg;
			break;

		case 'k':
			release = false;
			break;

		case 't':
			sol_timeout = atoi(optarg);
			break;

		case 'm':
			ra_holdoff_interval = atoi(optarg);
			break;

		case 'L':
			ra_options &= ~RA_RDNSS_DEFAULT_LIFETIME;
			break;

		case 'e':
			logopt |= LOG_PERROR;
			break;

		case 'd':
			daemonize = true;
			break;

		case 'p':
			pidfile = optarg;
			break;

		case 'f':
			client_options &= ~DHCPV6_CLIENT_FQDN;
			break;

		case 'a':
			client_options &= ~DHCPV6_ACCEPT_RECONFIGURE;
			break;

		case 'v':
			++verbosity;
			break;

		case 'x':
			res = parse_opt(optarg);
			if (res) {
				if (res > 0)
					return res;

				help = true;
			}
			break;

		default:
			help = true;
			break;
		}
	}

	if (allow_slaac_only > 0)
		script_sync_delay = allow_slaac_only;

	openlog("odhcp6c", logopt, LOG_DAEMON);
	if (!verbosity)
		setlogmask(LOG_UPTO(LOG_WARNING));

	ifname = argv[optind];

	if (help || !ifname)
		return usage();

	signal(SIGIO, sighandler);
	signal(SIGHUP, sighandler);
	signal(SIGINT, sighandler);
	signal(SIGTERM, sighandler);
	signal(SIGUSR1, sighandler);
	signal(SIGUSR2, sighandler);

	if ((urandom_fd = open("/dev/urandom", O_CLOEXEC | O_RDONLY)) < 0 ||
			init_dhcpv6(ifname, client_options, sol_timeout) ||
			ra_init(ifname, &ifid, ra_options, ra_holdoff_interval) ||
			script_init(script, ifname)) {
		syslog(LOG_ERR, "failed to initialize: %s", strerror(errno));
		return 3;
	}

	if (daemonize) {
		openlog("odhcp6c", LOG_PID, LOG_DAEMON); // Disable LOG_PERROR
		if (daemon(0, 0)) {
			syslog(LOG_ERR, "Failed to daemonize: %s",
					strerror(errno));
			return 4;
		}

		if (!pidfile) {
			snprintf((char*)buf, sizeof(buf), "/var/run/odhcp6c.%s.pid", ifname);
			pidfile = (char*)buf;
		}

		FILE *fp = fopen(pidfile, "w");
		if (fp) {
			fprintf(fp, "%i\n", getpid());
			fclose(fp);
		}
	}

	script_call("started", 0, false);

	while (!signal_term) { // Main logic
		odhcp6c_clear_state(STATE_SERVER_ID);
		odhcp6c_clear_state(STATE_SERVER_ADDR);
		odhcp6c_clear_state(STATE_IA_NA);
		odhcp6c_clear_state(STATE_IA_PD);
		odhcp6c_clear_state(STATE_SNTP_IP);
		odhcp6c_clear_state(STATE_NTP_IP);
		odhcp6c_clear_state(STATE_NTP_FQDN);
		odhcp6c_clear_state(STATE_SIP_IP);
		odhcp6c_clear_state(STATE_SIP_FQDN);
		bound = false;

		syslog(LOG_NOTICE, "(re)starting transaction on %s", ifname);

		signal_usr1 = signal_usr2 = false;
		int mode = dhcpv6_set_ia_mode(ia_na_mode, ia_pd_mode);
		if (mode != DHCPV6_STATELESS)
			mode = dhcpv6_request(DHCPV6_MSG_SOLICIT);

		odhcp6c_signal_process();

		if (mode < 0)
			continue;

		do {
			res = dhcpv6_request(mode == DHCPV6_STATELESS ?
					DHCPV6_MSG_INFO_REQ : DHCPV6_MSG_REQUEST);
			bool signalled = odhcp6c_signal_process();

			if (res > 0)
				break;
			else if (signalled) {
				mode = -1;
				break;
			}

			mode = dhcpv6_promote_server_cand();
		} while (mode > DHCPV6_UNKNOWN);

		if (mode < 0)
			continue;

		switch (mode) {
		case DHCPV6_STATELESS:
			bound = true;
			syslog(LOG_NOTICE, "entering stateless-mode on %s", ifname);

			while (!signal_usr2 && !signal_term) {
				signal_usr1 = false;
				script_call("informed", script_sync_delay, true);

				res = dhcpv6_poll_reconfigure();
				odhcp6c_signal_process();

				if (res > 0)
					continue;

				if (signal_usr1) {
					signal_usr1 = false; // Acknowledged
					continue;
				}

				if (signal_usr2 || signal_term)
					break;

				res = dhcpv6_request(DHCPV6_MSG_INFO_REQ);
				odhcp6c_signal_process();

				if (signal_usr1)
					continue;
				else if (res < 0)
					break;
			}
			break;

		case DHCPV6_STATEFUL:
			bound = true;
			script_call("bound", script_sync_delay, true);
			syslog(LOG_NOTICE, "entering stateful-mode on %s", ifname);

			while (!signal_usr2 && !signal_term) {
				// Renew Cycle
				// Wait for T1 to expire or until we get a reconfigure
				res = dhcpv6_poll_reconfigure();
				odhcp6c_signal_process();
				if (res > 0) {
					script_call("updated", 0, false);
					continue;
				}

				// Handle signal, if necessary
				if (signal_usr1)
					signal_usr1 = false; // Acknowledged

				if (signal_usr2 || signal_term)
					break; // Other signal type

				// Send renew as T1 expired
				res = dhcpv6_request(DHCPV6_MSG_RENEW);
				odhcp6c_signal_process();

				if (res > 0) { // Renew was succesfull
					// Publish updates
					script_call("updated", 0, false);
					continue; // Renew was successful
				}

				odhcp6c_clear_state(STATE_SERVER_ID); // Remove binding
				odhcp6c_clear_state(STATE_SERVER_ADDR);

				size_t ia_pd_len, ia_na_len;
				odhcp6c_get_state(STATE_IA_PD, &ia_pd_len);
				odhcp6c_get_state(STATE_IA_NA, &ia_na_len);

				if (ia_pd_len == 0 && ia_na_len == 0)
					break;

				// If we have IAs, try rebind otherwise restart
				res = dhcpv6_request(DHCPV6_MSG_REBIND);
				odhcp6c_signal_process();

				if (res > 0)
					script_call("rebound", 0, true);
				else
					break;
			}
			break;

		default:
			break;
		}

		odhcp6c_expire();

		size_t ia_pd_len, ia_na_len, server_id_len;
		odhcp6c_get_state(STATE_IA_PD, &ia_pd_len);
		odhcp6c_get_state(STATE_IA_NA, &ia_na_len);
		odhcp6c_get_state(STATE_SERVER_ID, &server_id_len);

		// Add all prefixes to lost prefixes
		bound = false;
		script_call("unbound", 0, true);

		if (server_id_len > 0 && (ia_pd_len > 0 || ia_na_len > 0) && release)
			dhcpv6_request(DHCPV6_MSG_RELEASE);

		odhcp6c_clear_state(STATE_IA_NA);
		odhcp6c_clear_state(STATE_IA_PD);
	}

	script_call("stopped", 0, true);

	return 0;
}

static int usage(void)
{
	const char buf[] =
	"Usage: odhcp6c [options] <interface>\n"
	"\nFeature options:\n"
	"	-S <time>	Wait at least <time> sec for a DHCP-server (0)\n"
	"	-N <mode>	Mode for requesting addresses [try|force|none]\n"
	"	-P <length>	Request IPv6-Prefix (0 = auto)\n"
	"	-F		Force IPv6-Prefix\n"
	"	-V <class>	Set vendor-class option (base-16 encoded)\n"
	"	-u <user-class> Set user-class option string\n"
	"	-x <opt>:<val>	Add option opt (with value val) in sent packets (cumulative)\n"
	"			Examples of IPv6 address, string and base-16 encoded options:\n"
	"			-x dns:2001:2001::1,2001:2001::2 - option 23\n"
	"			-x 15:office - option 15 (userclass)\n"
	"			-x 0x1f4:ABBA - option 500\n"
	"			-x 202:'\"file\"' - option 202\n"
	"	-c <clientid>	Override client-ID (base-16 encoded 16-bit type + value)\n"
	"	-i <iface-id>	Use a custom interface identifier for RA handling\n"
	"	-r <options>	Options to be requested (comma-separated)\n"
	"	-R		Do not request any options except those specified with -r\n"
	"	-s <script>	Status update script (/usr/sbin/odhcp6c-update)\n"
	"	-a		Don't send Accept Reconfigure option\n"
	"	-f		Don't send Client FQDN option\n"
	"	-k		Don't send a RELEASE when stopping\n"
	"	-t <seconds>	Maximum timeout for DHCPv6-SOLICIT (120)\n"
	"	-m <seconds>	Minimum time between accepting RA updates (3)\n"
	"	-L		Ignore default lifetime for RDNSS records\n"
	"	-U		Ignore Server Unicast option\n"
	"\nInvocation options:\n"
	"	-p <pidfile>	Set pidfile (/var/run/odhcp6c.pid)\n"
	"	-d		Daemonize\n"
	"	-e		Write logmessages to stderr\n"
	"	-v		Increase logging verbosity\n"
	"	-h		Show this help\n\n";
	fputs(buf, stderr);

	return 1;
}

// Don't want to pull-in librt and libpthread just for a monotonic clock...
uint64_t odhcp6c_get_milli_time(void)
{
	struct timespec t = {0, 0};
	syscall(SYS_clock_gettime, CLOCK_MONOTONIC, &t);

	return ((uint64_t)t.tv_sec) * 1000 + ((uint64_t)t.tv_nsec) / 1000000;
}

static uint8_t* odhcp6c_resize_state(enum odhcp6c_state state, ssize_t len)
{
	if (len == 0)
		return state_data[state] + state_len[state];
	else if (state_len[state] + len > 1024)
		return NULL;

	uint8_t *n = realloc(state_data[state], state_len[state] + len);

	if (n || state_len[state] + len == 0) {
		state_data[state] = n;
		n += state_len[state];
		state_len[state] += len;
	}

	return n;
}

bool odhcp6c_signal_process(void)
{
	while (signal_io) {
		signal_io = false;

		bool ra_updated = ra_process();

		if (ra_link_up()) {
			signal_usr2 = true;
			ra = false;
		}

		if (ra_updated && (bound || allow_slaac_only >= 0)) {
			script_call("ra-updated", (!ra && !bound) ?
					script_sync_delay : script_accu_delay, false);
			ra = true;
		}
	}

	return signal_usr1 || signal_usr2 || signal_term;
}

void odhcp6c_clear_state(enum odhcp6c_state state)
{
	state_len[state] = 0;
}

int odhcp6c_add_state(enum odhcp6c_state state, const void *data, size_t len)
{
	uint8_t *n = odhcp6c_resize_state(state, len);

	if (!n)
		return -1;

	memcpy(n, data, len);

	return 0;
}

int odhcp6c_insert_state(enum odhcp6c_state state, size_t offset, const void *data, size_t len)
{
	ssize_t len_after = state_len[state] - offset;
	if (len_after < 0)
		return -1;

	uint8_t *n = odhcp6c_resize_state(state, len);

	if (n) {
		uint8_t *sdata = state_data[state];

		memmove(sdata + offset + len, sdata + offset, len_after);
		memcpy(sdata + offset, data, len);
	}

	return 0;
}

size_t odhcp6c_remove_state(enum odhcp6c_state state, size_t offset, size_t len)
{
	uint8_t *data = state_data[state];
	ssize_t len_after = state_len[state] - (offset + len);

	if (len_after < 0)
		return state_len[state];

	memmove(data + offset, data + offset + len, len_after);

	return state_len[state] -= len;
}

void* odhcp6c_move_state(enum odhcp6c_state state, size_t *len)
{
	*len = state_len[state];
	void *data = state_data[state];

	state_len[state] = 0;
	state_data[state] = NULL;

	return data;
}

void* odhcp6c_get_state(enum odhcp6c_state state, size_t *len)
{
	*len = state_len[state];

	return state_data[state];
}

static struct odhcp6c_entry* odhcp6c_find_entry(enum odhcp6c_state state, const struct odhcp6c_entry *new)
{
	size_t len, cmplen = offsetof(struct odhcp6c_entry, target) + ((new->length + 7) / 8);
	uint8_t *start = odhcp6c_get_state(state, &len);

	for (struct odhcp6c_entry *c = (struct odhcp6c_entry*)start;
			(uint8_t*)c < &start[len] &&
			(uint8_t*)odhcp6c_next_entry(c) <= &start[len];
			c = odhcp6c_next_entry(c)) {
		if (!memcmp(c, new, cmplen) && !memcmp(c->auxtarget, new->auxtarget, new->auxlen))
			return c;
	}

	return NULL;
}

bool odhcp6c_update_entry(enum odhcp6c_state state, struct odhcp6c_entry *new,
		uint32_t safe, unsigned int holdoff_interval)
{
	size_t len;
	struct odhcp6c_entry *x = odhcp6c_find_entry(state, new);
	uint8_t *start = odhcp6c_get_state(state, &len);

	if (x && x->valid > new->valid && new->valid < safe)
		new->valid = safe;

	if (new->valid > 0) {
		if (x) {
			if (holdoff_interval && new->valid >= x->valid &&
					new->valid != UINT32_MAX &&
					new->valid - x->valid < holdoff_interval &&
					new->preferred >= x->preferred &&
					new->preferred != UINT32_MAX &&
					new->preferred - x->preferred < holdoff_interval)
				return false;

			x->valid = new->valid;
			x->preferred = new->preferred;
			x->t1 = new->t1;
			x->t2 = new->t2;
			x->iaid = new->iaid;
		} else if (odhcp6c_add_state(state, new, odhcp6c_entry_size(new)))
			return false;
	} else if (x)
		odhcp6c_remove_state(state, ((uint8_t*)x) - start, odhcp6c_entry_size(x));

	return true;
}

static void odhcp6c_expire_list(enum odhcp6c_state state, uint32_t elapsed)
{
	size_t len;
	uint8_t *start = odhcp6c_get_state(state, &len);

	for (struct odhcp6c_entry *c = (struct odhcp6c_entry*)start;
			(uint8_t*)c < &start[len] &&
			(uint8_t*)odhcp6c_next_entry(c) <= &start[len];
			) {
		if (c->t1 < elapsed)
			c->t1 = 0;
		else if (c->t1 != UINT32_MAX)
			c->t1 -= elapsed;

		if (c->t2 < elapsed)
			c->t2 = 0;
		else if (c->t2 != UINT32_MAX)
			c->t2 -= elapsed;

		if (c->preferred < elapsed)
			c->preferred = 0;
		else if (c->preferred != UINT32_MAX)
			c->preferred -= elapsed;

		if (c->valid < elapsed)
			c->valid = 0;
		else if (c->valid != UINT32_MAX)
			c->valid -= elapsed;

		if (!c->valid) {
			odhcp6c_remove_state(state, ((uint8_t*)c) - start, odhcp6c_entry_size(c));
			start = odhcp6c_get_state(state, &len);
		} else
			c = odhcp6c_next_entry(c);
	}
}

static uint8_t *odhcp6c_state_find_opt(const uint16_t code)
{
	size_t opts_len;
	uint8_t *odata, *opts = odhcp6c_get_state(STATE_OPTS, &opts_len);
	uint16_t otype, olen;

	dhcpv6_for_each_option(opts, &opts[opts_len], otype, olen, odata) {
		if (otype == code)
			return &odata[-4];
	}

	return NULL;
}

void odhcp6c_expire(void)
{
	time_t now = odhcp6c_get_milli_time() / 1000;
	uint32_t elapsed = (last_update > 0) ? now - last_update : 0;

	last_update = now;

	odhcp6c_expire_list(STATE_RA_PREFIX, elapsed);
	odhcp6c_expire_list(STATE_RA_ROUTE, elapsed);
	odhcp6c_expire_list(STATE_RA_DNS, elapsed);
	odhcp6c_expire_list(STATE_RA_SEARCH, elapsed);
	odhcp6c_expire_list(STATE_IA_NA, elapsed);
	odhcp6c_expire_list(STATE_IA_PD, elapsed);
}

uint32_t odhcp6c_elapsed(void)
{
	return odhcp6c_get_milli_time() / 1000 - last_update;
}

int odhcp6c_random(void *buf, size_t len)
{
	return read(urandom_fd, buf, len);
}

bool odhcp6c_is_bound(void)
{
	return bound;
}

bool odhcp6c_addr_in_scope(const struct in6_addr *addr)
{
	FILE *fd = fopen("/proc/net/if_inet6", "r");
	int len;
	bool ret = false;
	char buf[256];

	if (fd == NULL)
		return false;

	while (fgets(buf, sizeof(buf), fd)) {
		struct in6_addr inet6_addr;
		uint32_t flags, dummy;
		unsigned int i;
		char name[IF_NAMESIZE], addr_buf[33];

		len = strlen(buf);

		if ((len <= 0) || buf[len - 1] != '\n')
			break;

		buf[--len] = '\0';

		if (sscanf(buf, "%s %x %x %x %x %s",
				addr_buf, &dummy, &dummy, &dummy, &flags, name) != 6)
			break;

		if (strcmp(name, ifname) ||
			(flags & (IFA_F_DADFAILED | IFA_F_TENTATIVE | IFA_F_DEPRECATED)))
			continue;

		for (i = 0; i < strlen(addr_buf); i++) {
			if (!isxdigit(addr_buf[i]) || isupper(addr_buf[i]))
				break;
		}

		memset(&inet6_addr, 0, sizeof(inet6_addr));
		for (i = 0; i < (strlen(addr_buf) / 2); i++) {
			unsigned char byte;
			static const char hex[] = "0123456789abcdef";
			byte = ((index(hex, addr_buf[i * 2]) - hex) << 4) |
				(index(hex, addr_buf[i * 2 + 1]) - hex);
			inet6_addr.s6_addr[i] = byte;
		}

		if ((IN6_IS_ADDR_LINKLOCAL(&inet6_addr) == IN6_IS_ADDR_LINKLOCAL(addr)) &&
			(IN6_IS_ADDR_UNIQUELOCAL(&inet6_addr) == IN6_IS_ADDR_UNIQUELOCAL(addr))) {
			ret = true;
			break;
		}
	}

	fclose(fd);
	return ret;
}

static void sighandler(int signal)
{
	if (signal == SIGUSR1)
		signal_usr1 = true;
	else if (signal == SIGUSR2)
		signal_usr2 = true;
	else if (signal == SIGIO)
		signal_io = true;
	else
		signal_term = true;
}

static int add_opt(const uint16_t code, const uint8_t *data, const uint16_t len)
{
	struct {
		uint16_t code;
		uint16_t len;
	} opt_hdr = { htons(code), htons(len) };

	if (odhcp6c_state_find_opt(code))
		return -1;

	if (odhcp6c_add_state(STATE_OPTS, &opt_hdr, sizeof(opt_hdr)) ||
			odhcp6c_add_state(STATE_OPTS, data, len)) {
		syslog(LOG_ERR, "Failed to add option %hu", code);
		return 1;
	}

	return 0;
}

struct odhcp6c_opt *odhcp6c_find_opt(const uint16_t code)
{
	struct odhcp6c_opt *opt = opts;

	while (opt->code) {
		if (opt->code == code)
			return opt;

		opt++;
	}

	return NULL;
}

static struct odhcp6c_opt *odhcp6c_find_opt_by_name(const char *name)
{
	struct odhcp6c_opt *opt = opts;

	if (!name || !strlen(name))
		return NULL;

	while (opt->code && (!opt->str || strcmp(opt->str, name)))
		opt++;

	return (opt->code > 0 ? opt : NULL);
}

/* Find first occurrence of any character in the string <needles>
 * within the string <haystack>
 * */
static char *get_sep_pos(const char *haystack, const char *needles)
{
	unsigned int i;
	char *first = NULL;

	for (i = 0; i < strlen(needles); i++) {
		char *found = strchr(haystack, needles[i]);
		if (found && ((found < first) || (first == NULL)))
			first = found;
	}

	return first;
}

static int parse_opt_u8(const char *src, uint8_t **dst)
{
	int len = strlen(src);

	*dst = realloc(*dst, len/2);
	if (!*dst)
		return -1;

	return script_unhexlify(*dst, len, src);
}

static int parse_opt_string(const char *src, uint8_t **dst, const bool array)
{
	int o_len = 0;
	char *sep = get_sep_pos(src, ARRAY_SEP);

	if (sep && !array)
		return -1;

	do {
		if (sep) {
			*sep = 0;
			sep++;
		}

		int len = strlen(src);

		*dst = realloc(*dst, o_len + len);
		if (!*dst)
			return -1;

		memcpy(&((*dst)[o_len]), src, len);

		o_len += len;
		src = sep;

		if (sep)
			sep = get_sep_pos(src, ARRAY_SEP);
	} while (src);

	return o_len;
}

static int parse_opt_dns_string(const char *src, uint8_t **dst, const bool array)
{
	int o_len = 0;
	char *sep = get_sep_pos(src, ARRAY_SEP);

	if (sep && !array)
		return -1;

	do {
		uint8_t tmp[256];

		if (sep) {
			*sep = 0;
			sep++;
		}

		int len = dn_comp(src, tmp, sizeof(tmp), NULL, NULL);
		if (len < 0)
			return -1;

		*dst = realloc(*dst, o_len + len);
		if (!*dst)
			return -1;

		memcpy(&((*dst)[o_len]), tmp, len);

		o_len += len;
		src = sep;

		if (sep)
			sep = get_sep_pos(src, ARRAY_SEP);
	} while (src);

	return o_len;
}

static int parse_opt_ip6(const char *src, uint8_t **dst, const bool array)
{
	int o_len = 0;
	char *sep = get_sep_pos(src, ARRAY_SEP);

	if (sep && !array)
		return -1;

	do {
		int len = sizeof(struct in6_addr);

		if (sep) {
			*sep = 0;
			sep++;
		}

		*dst = realloc(*dst, o_len + len);
		if (!*dst)
			return -1;

		if (inet_pton(AF_INET6, src, &((*dst)[o_len])) < 1)
			return -1;

		o_len += len;
		src = sep;

		if (sep)
			sep = get_sep_pos(src, ARRAY_SEP);
	} while (src);

	return o_len;
}

static int parse_opt_user_class(const char *src, uint8_t **dst, const bool array)
{
	int o_len = 0;
	char *sep = get_sep_pos(src, ARRAY_SEP);

	if (sep && !array)
		return -1;

	do {
		if (sep) {
			*sep = 0;
			sep++;
		}
		uint16_t str_len = strlen(src);

		*dst = realloc(*dst, o_len + str_len + 2);
		if (!*dst)
			return -1;

		struct user_class {
			uint16_t len;
			uint8_t data[];
		} *e = (struct user_class *)&((*dst)[o_len]);

		e->len = ntohs(str_len);
		memcpy(e->data, src, str_len);

		o_len += str_len + 2;
		src = sep;

		if (sep)
			sep = get_sep_pos(src, ARRAY_SEP);
	} while (src);

	return o_len;
}

static int parse_opt_data(const char *data, uint8_t **dst, const unsigned int type,
		const bool array)
{
	int ret = 0;

	switch (type) {
	case OPT_U8:
		ret = parse_opt_u8(data, dst);
		break;

	case OPT_STR:
		ret = parse_opt_string(data, dst, array);
		break;

	case OPT_DNS_STR:
		ret = parse_opt_dns_string(data, dst, array);
		break;

	case OPT_IP6:
		ret = parse_opt_ip6(data, dst, array);
		break;

	case OPT_USER_CLASS:
		ret = parse_opt_user_class(data, dst, array);
		break;

	default:
		ret = -1;
		break;
	}

	return ret;
}

static int parse_opt(const char *opt)
{
	uint32_t optn;
	char *data;
	uint8_t *payload = NULL;
	int payload_len;
	unsigned int type = OPT_U8;
	bool array = false;
	struct odhcp6c_opt *dopt = NULL;
	int ret = -1;

	data = get_sep_pos(opt, ":");
	if (!data)
		return -1;

	*data = '\0';
	data++;

	if (strlen(opt) == 0 || strlen(data) == 0)
		return -1;

	dopt = odhcp6c_find_opt_by_name(opt);
	if (!dopt) {
		char *e;
		optn = strtoul(opt, &e, 0);
		if (*e || e == opt || optn > USHRT_MAX)
			return -1;

		dopt = odhcp6c_find_opt(optn);
	} else
		optn = dopt->code;

	/* Check if the type for the content is well-known */
	if (dopt) {
		/* Refuse internal options */
		if (dopt->flags & OPT_INTERNAL)
			return -1;

		type = dopt->flags & OPT_MASK_SIZE;
		array = ((dopt->flags & OPT_ARRAY) == OPT_ARRAY) ? true : false;
	} else if (data[0] == '"' || data[0] == '\'') {
		char *end = strrchr(data + 1, data[0]);

		if (end && (end == (data + strlen(data) - 1))) {
			/* Raw option is specified as a string */
			type = OPT_STR;
			data++;
			*end = '\0';
		}

	}

	payload_len = parse_opt_data(data, &payload, type, array);
	if (payload_len > 0)
		ret = add_opt(optn, payload, payload_len);

	free(payload);

	return ret;
}
