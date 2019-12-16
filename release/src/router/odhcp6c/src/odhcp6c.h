/**
 * Copyright (C) 2012-2014 Steven Barth <steven@midlink.org>
 * Copyright (C) 2018 Hans Dedecker <dedeckeh@gmail.com>
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
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <netinet/in.h>

#include "stubs.h"

#define _unused __attribute__((unused))
#define _packed __attribute__((packed))
#define _aligned(n) __attribute__((aligned(n)))

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define ND_OPT_RECURSIVE_DNS 25
#define ND_OPT_DNSSL 31

#define DHCPV6_SOL_MAX_RT 120
#define DHCPV6_REQ_MAX_RT 30
#define DHCPV6_CNF_MAX_RT 4
#define DHCPV6_REN_MAX_RT 600
#define DHCPV6_REB_MAX_RT 600
#define DHCPV6_INF_MAX_RT 120

#define RA_MIN_ADV_INTERVAL 3   /* RFC 4861 paragraph 6.2.1 */

enum dhcvp6_opt {
	DHCPV6_OPT_CLIENTID = 1,
	DHCPV6_OPT_SERVERID = 2,
	DHCPV6_OPT_IA_NA = 3,
	DHCPV6_OPT_IA_TA = 4,
	DHCPV6_OPT_IA_ADDR = 5,
	DHCPV6_OPT_ORO = 6,
	DHCPV6_OPT_PREF = 7,
	DHCPV6_OPT_ELAPSED = 8,
	DHCPV6_OPT_RELAY_MSG = 9,
	DHCPV6_OPT_AUTH = 11,
	DHCPV6_OPT_UNICAST = 12,
	DHCPV6_OPT_STATUS = 13,
	DHCPV6_OPT_RAPID_COMMIT = 14,
	DHCPV6_OPT_USER_CLASS = 15,
	DHCPV6_OPT_VENDOR_CLASS = 16,
	DHCPV6_OPT_INTERFACE_ID = 18,
	DHCPV6_OPT_RECONF_MESSAGE = 19,
	DHCPV6_OPT_RECONF_ACCEPT = 20,
	DHCPV6_OPT_SIP_SERVER_D = 21,
	DHCPV6_OPT_SIP_SERVER_A = 22,
	DHCPV6_OPT_DNS_SERVERS = 23,
	DHCPV6_OPT_DNS_DOMAIN = 24,
	DHCPV6_OPT_IA_PD = 25,
	DHCPV6_OPT_IA_PREFIX = 26,
	DHCPV6_OPT_SNTP_SERVERS = 31,
	DHCPV6_OPT_INFO_REFRESH = 32,
	DHCPV6_OPT_REMOTE_ID = 37,
	DHCPV6_OPT_SUBSCRIBER_ID = 38,
	DHCPV6_OPT_FQDN = 39,
	DHCPV6_OPT_ERO = 43,
	DHCPV6_OPT_LQ_QUERY = 44,
	DHCPV6_OPT_CLIENT_DATA = 45,
	DHCPV6_OPT_CLT_TIME = 46,
	DHCPV6_OPT_LQ_RELAY_DATA = 47,
	DHCPV6_OPT_LQ_CLIENT_LINK = 48,
	DHCPV6_OPT_RELAY_ID = 53,
	DHCPV6_OPT_NTP_SERVER = 56,
	DHCPV6_OPT_CLIENT_ARCH_TYPE = 61,
	DHCPV6_OPT_AFTR_NAME = 64,
	DHCPV6_OPT_RSOO = 66,
	DHCPV6_OPT_PD_EXCLUDE = 67,
	DHCPV6_OPT_VSS = 68,
	DHCPV6_OPT_LINK_LAYER_ADDRESS = 79,
	DHCPV6_OPT_LINK_ADDRESS = 80,
	DHCPV6_OPT_RADIUS = 81,
	DHCPV6_OPT_SOL_MAX_RT = 82,
	DHCPV6_OPT_INF_MAX_RT = 83,
#ifdef EXT_CER_ID
	/* draft-donley-dhc-cer-id-option-03 */
	DHCPV6_OPT_CER_ID = EXT_CER_ID,
#endif
	DHCPV6_OPT_DHCPV4_MSG = 87,
	/* draft-ietf-softwire-map-dhcp-08 */
	DHCPV6_OPT_S46_RULE = 89,
	DHCPV6_OPT_S46_BR = 90,
	DHCPV6_OPT_S46_DMR = 91,
	DHCPV6_OPT_S46_V4V6BIND = 92,
	DHCPV6_OPT_S46_PORTPARAMS = 93,
	DHCPV6_OPT_S46_CONT_MAPE = 94,
	DHCPV6_OPT_S46_CONT_MAPT = 95,
	DHCPV6_OPT_S46_CONT_LW = 96,
	DHCPV6_OPT_LQ_BASE_TIME = 100,
	DHCPV6_OPT_LQ_START_TIME = 101,
	DHCPV6_OPT_LQ_END_TIME = 102,
	DHCPV6_OPT_ANI_ATT = 105,
	DHCPV6_OPT_ANI_NETWORK_NAME = 106,
	DHCPV6_OPT_ANI_AP_NAME = 107,
	DHCPV6_OPT_ANI_AP_BSSID = 108,
	DHCPV6_OPT_ANI_OPERATOR_ID = 109,
	DHCPV6_OPT_ANI_OPERATOR_REALM = 110,
	DHCPV6_OPT_MUD_URL_V6 = 112,
	DHCPV6_OPT_F_BINDING_STATUS = 114,
	DHCPV6_OPT_F_CONNECT_FLAGS = 115,
	DHCPV6_OPT_F_DNS_REMOVAL_INFO = 116,
	DHCPV6_OPT_F_DNS_HOST_NAME = 117,
	DHCPV6_OPT_F_DNS_ZONE_NAME = 118,
	DHCPV6_OPT_F_DNS_FLAGS = 119,
	DHCPV6_OPT_F_EXPIRATION_TIME = 120,
	DHCPV6_OPT_F_MAX_UNACKED_BNDUPD = 121,
	DHCPV6_OPT_F_MCLT = 122,
	DHCPV6_OPT_F_PARTNER_LIFETIME = 123,
	DHCPV6_OPT_F_PARTNER_LIFETIME_SENT = 124,
	DHCPV6_OPT_F_PARTNER_DOWN_TIME = 125,
	DHCPV6_OPT_F_PARTNER_RAW_CLT_TIME = 126,
	DHCPV6_OPT_F_PROTOCOL_VERSION = 127,
	DHCPV6_OPT_F_KEEPALIVE_TIME = 128,
	DHCPV6_OPT_F_RECONFIGURE_DATA = 129,
	DHCPV6_OPT_F_RELATIONSHIP_NAME = 130,
	DHCPV6_OPT_F_SERVER_FLAGS = 131,
	DHCPV6_OPT_F_SERVER_STATE = 132,
	DHCPV6_OPT_F_START_TIME_OF_STATE = 133,
	DHCPV6_OPT_F_STATE_EXPIRATION_TIME  = 134,
	DHCPV6_OPT_RELAY_PORT = 135,
};

enum dhcpv6_opt_npt {
	NTP_SRV_ADDR = 1,
	NTP_MC_ADDR = 2,
	NTP_SRV_FQDN = 3
};

enum dhcpv6_msg {
	DHCPV6_MSG_UNKNOWN = 0,
	DHCPV6_MSG_SOLICIT = 1,
	DHCPV6_MSG_ADVERT = 2,
	DHCPV6_MSG_REQUEST = 3,
	DHCPV6_MSG_RENEW = 5,
	DHCPV6_MSG_REBIND = 6,
	DHCPV6_MSG_REPLY = 7,
	DHCPV6_MSG_RELEASE = 8,
	DHCPV6_MSG_DECLINE = 9,
	DHCPV6_MSG_RECONF = 10,
	DHCPV6_MSG_INFO_REQ = 11,
	_DHCPV6_MSG_MAX
};

enum dhcpv6_status {
	DHCPV6_Success = 0,
	DHCPV6_UnspecFail = 1,
	DHCPV6_NoAddrsAvail = 2,
	DHCPV6_NoBinding = 3,
	DHCPV6_NotOnLink = 4,
	DHCPV6_UseMulticast = 5,
	DHCPV6_NoPrefixAvail = 6,
	_DHCPV6_Status_Max
};

enum dhcpv6_config {
	DHCPV6_STRICT_OPTIONS = 1,
	DHCPV6_CLIENT_FQDN = 2,
	DHCPV6_ACCEPT_RECONFIGURE = 4,
	DHCPV6_IGNORE_OPT_UNICAST = 8,
};

typedef int(reply_handler)(enum dhcpv6_msg orig, const int rc,
		const void *opt, const void *end, const struct sockaddr_in6 *from);

// retransmission strategy
struct dhcpv6_retx {
	bool delay;
	uint8_t init_timeo;
	uint16_t max_timeo;
	uint8_t max_rc;
	char name[8];
	reply_handler *handler_reply;
	int(*handler_finish)(void);
};

// DHCPv6 Protocol Headers
struct dhcpv6_header {
	uint8_t msg_type;
	uint8_t tr_id[3];
} __attribute__((packed));

struct dhcpv6_ia_hdr {
	uint16_t type;
	uint16_t len;
	uint32_t iaid;
	uint32_t t1;
	uint32_t t2;
} _packed;

struct dhcpv6_ia_addr {
	uint16_t type;
	uint16_t len;
	struct in6_addr addr;
	uint32_t preferred;
	uint32_t valid;
} _packed;

struct dhcpv6_ia_prefix {
	uint16_t type;
	uint16_t len;
	uint32_t preferred;
	uint32_t valid;
	uint8_t prefix;
	struct in6_addr addr;
} _packed;

struct dhcpv6_duid {
	uint16_t type;
	uint16_t len;
	uint16_t duid_type;
	uint8_t data[128];
} _packed;

struct dhcpv6_auth_reconfigure {
	uint16_t type;
	uint16_t len;
	uint8_t protocol;
	uint8_t algorithm;
	uint8_t rdm;
	uint64_t replay;
	uint8_t reconf_type;
	uint8_t key[16];
} _packed;

struct dhcpv6_cer_id {
	uint16_t type;
	uint16_t len;
	struct in6_addr addr;
} _packed;

struct dhcpv6_s46_portparams {
	uint8_t offset;
	uint8_t psid_len;
	uint16_t psid;
} _packed;

struct dhcpv6_s46_v4v6bind {
	struct in_addr ipv4_address;
	uint8_t bindprefix6_len;
	uint8_t bind_ipv6_prefix[];
} _packed;

struct dhcpv6_s46_dmr {
	uint8_t dmr_prefix6_len;
	uint8_t dmr_ipv6_prefix[];
} _packed;

struct dhcpv6_s46_rule {
	uint8_t flags;
	uint8_t ea_len;
	uint8_t prefix4_len;
	struct in_addr ipv4_prefix;
	uint8_t prefix6_len;
	uint8_t ipv6_prefix[];
} _packed;

#define dhcpv6_for_each_option(start, end, otype, olen, odata)\
	for (uint8_t *_o = (uint8_t*)(start); _o + 4 <= (uint8_t*)(end) &&\
		((otype) = _o[0] << 8 | _o[1]) && ((odata) = (void*)&_o[4]) &&\
		((olen) = _o[2] << 8 | _o[3]) + (odata) <= (uint8_t*)(end); \
		_o += 4 + (_o[2] << 8 | _o[3]))


struct dhcpv6_server_cand {
	bool has_noaddravail;
	bool wants_reconfigure;
	int16_t preference;
	uint8_t duid_len;
	uint8_t duid[130];
	struct in6_addr server_addr;
	uint32_t sol_max_rt;
	uint32_t inf_max_rt;
	void *ia_na;
	void *ia_pd;
	size_t ia_na_len;
	size_t ia_pd_len;
};


enum odhcp6c_state {
	STATE_CLIENT_ID,
	STATE_SERVER_ID,
	STATE_SERVER_CAND,
	STATE_SERVER_ADDR,
	STATE_ORO,
	STATE_DNS,
	STATE_SEARCH,
	STATE_IA_NA,
	STATE_IA_PD,
	STATE_IA_PD_INIT,
	STATE_CUSTOM_OPTS,
	STATE_SNTP_IP,
	STATE_NTP_IP,
	STATE_NTP_FQDN,
	STATE_SIP_IP,
	STATE_SIP_FQDN,
	STATE_RA_ROUTE,
	STATE_RA_PREFIX,
	STATE_RA_DNS,
	STATE_RA_SEARCH,
	STATE_AFTR_NAME,
	STATE_OPTS,
	STATE_CER,
	STATE_S46_MAPT,
	STATE_S46_MAPE,
	STATE_S46_LW,
	STATE_PASSTHRU,
	_STATE_MAX
};


struct icmp6_opt {
	uint8_t type;
	uint8_t len;
	uint8_t data[6];
};


enum dhcpv6_mode {
	DHCPV6_UNKNOWN = -1,
	DHCPV6_STATELESS,
	DHCPV6_STATEFUL
};

enum ra_config {
	RA_RDNSS_DEFAULT_LIFETIME = 1,
};

enum odhcp6c_ia_mode {
	IA_MODE_NONE,
	IA_MODE_TRY,
	IA_MODE_FORCE,
};


struct odhcp6c_entry {
	struct in6_addr router;
	uint8_t auxlen;
	uint8_t length;
	int16_t priority;
	struct in6_addr target;
	uint32_t valid;
	uint32_t preferred;
	uint32_t t1;
	uint32_t t2;
	uint32_t iaid;
	uint8_t auxtarget[];
};

// Include padding after auxtarget to align the next entry
#define odhcp6c_entry_size(entry) \
	(sizeof(struct odhcp6c_entry) +	(((entry)->auxlen + 3) & ~3))

#define odhcp6c_next_entry(entry) \
	((struct odhcp6c_entry *)((uint8_t *)(entry) + odhcp6c_entry_size(entry)))


struct odhcp6c_request_prefix {
	uint32_t iaid;
	uint16_t length;
};

enum odhcp6c_opt_flags {
	OPT_U8 = 0,
	OPT_IP6,
	OPT_STR,
	OPT_DNS_STR,
	OPT_USER_CLASS,
	OPT_MASK_SIZE = 0x0F,
	OPT_ARRAY = 0x10,
	OPT_INTERNAL = 0x20,
	OPT_NO_PASSTHRU = 0x40,
	OPT_ORO = 0x80,
	OPT_ORO_STATEFUL = 0x100,
	OPT_ORO_STATELESS = 0x200,
};

struct odhcp6c_opt {
	uint16_t code;
	uint16_t flags;
	const char *str;
};

int init_dhcpv6(const char *ifname, unsigned int client_options, int sol_timeout);
int dhcpv6_set_ia_mode(enum odhcp6c_ia_mode na, enum odhcp6c_ia_mode pd);
int dhcpv6_request(enum dhcpv6_msg type);
int dhcpv6_poll_reconfigure(void);
int dhcpv6_promote_server_cand(void);

int init_rtnetlink(void);
int set_rtnetlink_addr(int ifindex, const struct in6_addr *addr,
		uint32_t pref, uint32_t valid);

int ra_get_hoplimit(void);
int ra_get_mtu(void);
int ra_get_reachable(void);
int ra_get_retransmit(void);

int script_init(const char *path, const char *ifname);
ssize_t script_unhexlify(uint8_t *dst, size_t len, const char *src);
void script_call(const char *status, int delay, bool resume);

bool odhcp6c_signal_process(void);
uint64_t odhcp6c_get_milli_time(void);
int odhcp6c_random(void *buf, size_t len);
bool odhcp6c_is_bound(void);
bool odhcp6c_addr_in_scope(const struct in6_addr *addr);

// State manipulation
void odhcp6c_clear_state(enum odhcp6c_state state);
int odhcp6c_add_state(enum odhcp6c_state state, const void *data, size_t len);
void odhcp6c_append_state(enum odhcp6c_state state, const void *data, size_t len);
int odhcp6c_insert_state(enum odhcp6c_state state, size_t offset, const void *data, size_t len);
size_t odhcp6c_remove_state(enum odhcp6c_state state, size_t offset, size_t len);
void* odhcp6c_move_state(enum odhcp6c_state state, size_t *len);
void* odhcp6c_get_state(enum odhcp6c_state state, size_t *len);

// Entry manipulation
bool odhcp6c_update_entry(enum odhcp6c_state state, struct odhcp6c_entry *new,
				uint32_t safe, unsigned int holdoff_interval);

void odhcp6c_expire(void);
uint32_t odhcp6c_elapsed(void);
struct odhcp6c_opt *odhcp6c_find_opt(const uint16_t code);
