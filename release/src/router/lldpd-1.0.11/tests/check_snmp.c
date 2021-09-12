/* -*- mode: c; c-file-style: "openbsd" -*- */
/*
 * Copyright (c) 2015 Vincent Bernat <bernat@luffy.cx>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <check.h>

#include "../src/daemon/lldpd.h"
#include "../src/daemon/agent.h"

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/agent/snmp_vars.h>

extern struct lldpd *agent_scfg;
extern struct timeval starttime;
extern struct variable8 agent_lldp_vars[];

/* Our test config */
struct lldpd test_cfg = {
	.g_config = {
		.c_tx_interval = 30000,
		.c_tx_hold = 2,
		.c_ttl = 60,
		.c_smart = 0
	}
};
struct timeval test_starttime = { .tv_sec = 100, .tv_usec = 0 };

/* First chassis */
struct lldpd_mgmt mgmt1 = {
		.m_family = LLDPD_AF_IPV4,
		.m_addr = { .octets = { 0xc0, 0, 0x2, 0xf } }, /* 192.0.2.15 */
		.m_addrsize = sizeof(struct in_addr),
		.m_iface = 3
};
struct lldpd_chassis chassis1 = {
	.c_index         = 1,
	.c_protocol      = LLDPD_MODE_LLDP,
	.c_id_subtype    = LLDP_CHASSISID_SUBTYPE_LLADDR,
	.c_id            = "AAA012",
	.c_id_len        = 6,
	.c_name          = "chassis1.example.com",
	.c_descr         = "First chassis",
	.c_cap_available = LLDP_CAP_BRIDGE | LLDP_CAP_WLAN | LLDP_CAP_ROUTER,
	.c_cap_enabled   = LLDP_CAP_ROUTER,
#ifdef ENABLE_LLDPMED
	.c_med_cap_available = LLDP_MED_CAP_CAP | LLDP_MED_CAP_IV | \
		LLDP_MED_CAP_LOCATION |	LLDP_MED_CAP_POLICY | \
		LLDP_MED_CAP_MDI_PSE | LLDP_MED_CAP_MDI_PD,
	.c_med_type   = LLDP_MED_CLASS_II,
	.c_med_hw     = "Hardware 1",
	/* We skip c_med_fw */
	.c_med_sw     = "Software 1",
	.c_med_sn     = "00-00-0000-AAAA",
	.c_med_manuf  = "Manufacturer 1",
	.c_med_model  = "Model 1",
	.c_med_asset  = "Asset 1",
#endif
};
/* Second chassis */
struct lldpd_mgmt mgmt2 = {
		.m_family = LLDPD_AF_IPV4,
		.m_addr = { .octets = { 0xc0, 0, 0x2, 0x11 } }, /* 192.0.2.17 */
		.m_addrsize = sizeof(struct in_addr),
		.m_iface = 5
};
struct lldpd_mgmt mgmt3 = {
		.m_family = LLDPD_AF_IPV6,
		.m_addr = { .octets = { 0x20, 0x01, 0x0d, 0xb8,
					0xca, 0xfe, 0x00, 0x00,
					0x00, 0x00, 0x00, 0x00,
					0x00, 0x00, 0x00, 0x17 } }, /* 2001:db8:cafe::17 */
		.m_addrsize = sizeof(struct in6_addr),
		.m_iface = 5
};
struct lldpd_chassis chassis2 = {
	.c_index         = 4,
	.c_protocol      = LLDPD_MODE_LLDP,
	.c_id_subtype    = LLDP_CHASSISID_SUBTYPE_LOCAL,
	.c_id            = "chassis2",
	.c_id_len        = 6,
	.c_name          = "chassis2.example.com",
	.c_descr         = "Second chassis",
	.c_cap_available = LLDP_CAP_ROUTER,
	.c_cap_enabled   = LLDP_CAP_ROUTER,
#ifdef ENABLE_LLDPMED
	.c_med_hw     = "Hardware 2",
	/* We skip c_med_fw */
	.c_med_sw     = "Software 2",
	.c_med_sn     = "00-00-0000-AAAC",
	.c_med_manuf  = "Manufacturer 2",
	.c_med_model  = "Model 2",
	.c_med_asset  = "Asset 2",
#endif
};

/* First port of first chassis */
struct lldpd_hardware hardware1 = {
	.h_ifindex = 3,
	.h_tx_cnt = 1352,
	.h_rx_cnt = 1458,
	.h_rx_discarded_cnt = 5,
	.h_rx_unrecognized_cnt = 4,
	.h_insert_cnt = 100,
	.h_delete_cnt = 5,
	.h_ageout_cnt = 20,
	.h_drop_cnt = 1,
	.h_lport = {
		.p_chassis    = &chassis1,
		.p_lastchange = 200,
		.p_protocol   = LLDPD_MODE_LLDP,
		.p_id_subtype = LLDP_PORTID_SUBTYPE_LLADDR,
		.p_id         = "AAA012",
		.p_id_len     = 6,
		.p_descr      = "eth2",
		.p_mfs        = 1600,
#ifdef ENABLE_DOT3
		.p_aggregid   = 0,
		.p_macphy     = {
			.autoneg_support = 1,
			.autoneg_enabled = 1,
			.autoneg_advertised = LLDP_DOT3_LINK_AUTONEG_100BASE_TX | LLDP_DOT3_LINK_AUTONEG_100BASE_TXFD,
			.mau_type = LLDP_DOT3_MAU_100BASETXFD,
		},
		.p_power = {
			.devicetype  = LLDP_DOT3_POWER_PD,
			.supported   = 1,
			.enabled     = 1,
			.paircontrol = 1,
			.pairs       = 2,
			.class       = 3,
			.powertype   = LLDP_DOT3_POWER_8023AT_TYPE2,
			.source      = LLDP_DOT3_POWER_SOURCE_BOTH,
			.priority    = LLDP_DOT3_POWER_PRIO_LOW,
			.requested   = 2000,
			.allocated   = 2500,
		},
#endif
#ifdef ENABLE_LLDPMED
		.p_med_cap_enabled = LLDP_MED_CAP_CAP | LLDP_MED_CAP_IV | LLDP_MED_CAP_MDI_PD |
			LLDP_MED_CAP_POLICY | LLDP_MED_CAP_LOCATION,
		.p_med_policy = {
			{ .type = 0 }, { .type = 0 }, {
				.type = LLDP_MED_APPTYPE_GUESTVOICE,
				.unknown = 1,
				.tagged = 1,
				.vid = 475,
				.priority = 3,
				.dscp = 62
			}, { .type = 0 }, { .type = 0 }, { .type = 0 }, {
				.type = LLDP_MED_APPTYPE_VIDEOSTREAM,
				.unknown = 0,
				.tagged = 1,
				.vid = 472,
				.priority = 1,
				.dscp = 60
			}, { .type = 0 }
		},
		.p_med_location = {
			{ .format = 0 }, {
				.format = LLDP_MED_LOCFORMAT_CIVIC,
				/* 2:FR:6:Commercial Rd:19:4 */
				.data = "\x15" "\x02" "FR" "\x06" "\x0d" "Commercial Rd" "\x13" "\x01" "4",
				.data_len = 22,
			}, { .format = 0 }
		},
		.p_med_power = {
			.devicetype = LLDP_MED_POW_TYPE_PD,
			.source = LLDP_MED_POW_SOURCE_LOCAL,
			.priority = LLDP_MED_POW_PRIO_HIGH,
			.val = 100
		},
#endif		
#ifdef ENABLE_DOT1
		.p_pvid = 47,
		/* Remaining is done is snmp_config */
#endif
	}
};
/* Second port of first chassis */
struct lldpd_hardware hardware2 = {
	.h_ifindex = 4,
	.h_tx_cnt = 11352,
	.h_rx_cnt = 11458,
	.h_rx_discarded_cnt = 55,
	.h_rx_unrecognized_cnt = 14,
	.h_insert_cnt = 1000,
	.h_delete_cnt = 51,
	.h_ageout_cnt = 210,
	.h_drop_cnt = 1,
	.h_lport = {
		.p_chassis    = &chassis1,
		.p_lastchange = 50,
		.p_protocol   = LLDPD_MODE_LLDP,
		.p_id_subtype = LLDP_PORTID_SUBTYPE_IFNAME,
		.p_id         = "eth4",
		.p_id_len     = 4,
		.p_descr      = "Intel 1000 GE",
		.p_mfs        = 9000,
#ifdef ENABLE_DOT3
		.p_aggregid   = 3,
		.p_macphy     = {
			.autoneg_support = 1,
			.autoneg_enabled = 1,
			.autoneg_advertised = LLDP_DOT3_LINK_AUTONEG_100BASE_TXFD | LLDP_DOT3_LINK_AUTONEG_1000BASE_TFD,
			.mau_type = LLDP_DOT3_MAU_1000BASETFD,
		},
#endif
#ifdef ENABLE_LLDPMED
		.p_med_cap_enabled = LLDP_MED_CAP_CAP | LLDP_MED_CAP_IV | LLDP_MED_CAP_MDI_PD |
			LLDP_MED_CAP_MDI_PSE | LLDP_MED_CAP_POLICY | LLDP_MED_CAP_LOCATION,
		.p_med_policy = {
			{ .type = 0 }, { .type = 0 }, {
				.type = LLDP_MED_APPTYPE_GUESTVOICE,
				.unknown = 1,
				.tagged = 1,
				.vid = 475,
				.priority = 3,
				.dscp = 62
			}, { .type = 0 }, { .type = 0 }, {
				.type = LLDP_MED_APPTYPE_VIDEOCONFERENCE,
				.unknown = 0,
				.tagged = 0,
				.vid = 1007,
				.priority = 1,
				.dscp = 49
			}, { .type = 0 }, { .type = 0 }
		},
		.p_med_location = {
			{
				.format = LLDP_MED_LOCFORMAT_COORD,
				.data = "Not interpreted",
				.data_len = 15,
			}, { .format = 0 }, { .format = 0 },
		},
#endif
	}
};

#ifdef ENABLE_CUSTOM
struct lldpd_custom custom1 = {
	.oui = { 33, 44, 55 },
	.subtype = 44,
	.oui_info = (u_int8_t*)"OUI content",
};
struct lldpd_custom custom2 = {
	.oui = { 33, 44, 55 },
	.subtype = 44,
	.oui_info = (u_int8_t*)"More content",
};
struct lldpd_custom custom3 = {
	.oui = { 33, 44, 55 },
	.subtype = 45,
	.oui_info = (u_int8_t*)"More more content",
};
struct lldpd_custom custom4 = {
	.oui = { 33, 44, 56 },
	.subtype = 44,
	.oui_info = (u_int8_t*)"Even more content",
};
struct lldpd_custom custom5 = {
	.oui = { 33, 44, 55 },
	.subtype = 44,
	.oui_info = (u_int8_t*)"Still more content",
};
#endif

#ifdef ENABLE_DOT1
struct lldpd_vlan vlan47 = {
	.v_name = "VLAN #47",
	.v_vid = 47,
};
struct lldpd_vlan vlan49 = {
	.v_name = "VLAN #49",
	.v_vid = 49,
};
struct lldpd_vlan vlan1449 = {
	.v_name = "VLAN #1449",
	.v_vid = 1449,
};
struct lldpd_ppvid ppvid47 = {
	.p_cap_status = LLDP_PPVID_CAP_SUPPORTED | LLDP_PPVID_CAP_ENABLED,
	.p_ppvid = 47,
};
struct lldpd_ppvid ppvid118 = {
	.p_cap_status = LLDP_PPVID_CAP_SUPPORTED | LLDP_PPVID_CAP_ENABLED,
	.p_ppvid = 118,
};
struct lldpd_pi pi88cc = {
	.p_pi = "\x88\xcc",
	.p_pi_len = 2,
};
struct lldpd_pi pi888e01 = {
	.p_pi = "\x88\x8e\x01",
	.p_pi_len = 3,
};
#endif

/* First port of second chassis */
struct lldpd_port port2 = {
	.p_chassis    = &chassis2,
	.p_lastchange = 180,
	.p_protocol   = LLDPD_MODE_LLDP,
	.p_id_subtype = LLDP_PORTID_SUBTYPE_IFALIAS,
	.p_id         = "Giga1/7",
	.p_id_len     = 7,
	.p_descr      = "Gigabit Ethernet 1/7",
};

void
snmp_config()
{
	starttime = test_starttime;
	agent_scfg = &test_cfg;
	TAILQ_INIT(&test_cfg.g_chassis);
	TAILQ_INIT(&chassis1.c_mgmt);
	TAILQ_INSERT_TAIL(&chassis1.c_mgmt, &mgmt1, m_entries);
	TAILQ_INSERT_TAIL(&test_cfg.g_chassis, &chassis1, c_entries);
	TAILQ_INIT(&chassis2.c_mgmt);
	TAILQ_INSERT_TAIL(&chassis2.c_mgmt, &mgmt2, m_entries);
	TAILQ_INSERT_TAIL(&chassis2.c_mgmt, &mgmt3, m_entries);
	TAILQ_INSERT_TAIL(&test_cfg.g_chassis, &chassis2, c_entries);
	TAILQ_INIT(&test_cfg.g_hardware);
	TAILQ_INSERT_TAIL(&test_cfg.g_hardware, &hardware1, h_entries);
	TAILQ_INSERT_TAIL(&test_cfg.g_hardware, &hardware2, h_entries);
#ifdef ENABLE_CUSTOM
	custom1.oui_info_len = strlen((char*)custom1.oui_info);
	custom2.oui_info_len = strlen((char*)custom2.oui_info);
	custom3.oui_info_len = strlen((char*)custom3.oui_info);
	custom4.oui_info_len = strlen((char*)custom4.oui_info);
	custom5.oui_info_len = strlen((char*)custom5.oui_info);
	TAILQ_INIT(&hardware1.h_lport.p_custom_list);
	TAILQ_INIT(&hardware2.h_lport.p_custom_list);
	TAILQ_INIT(&port2.p_custom_list);
	TAILQ_INSERT_TAIL(&hardware2.h_lport.p_custom_list, &custom1, next);
	TAILQ_INSERT_TAIL(&hardware2.h_lport.p_custom_list, &custom2, next);
	TAILQ_INSERT_TAIL(&hardware2.h_lport.p_custom_list, &custom3, next);
	TAILQ_INSERT_TAIL(&hardware2.h_lport.p_custom_list, &custom4, next);
	TAILQ_INSERT_TAIL(&hardware1.h_lport.p_custom_list, &custom5, next);
#endif
#ifdef ENABLE_DOT1
	TAILQ_INIT(&hardware1.h_lport.p_vlans);
	TAILQ_INSERT_TAIL(&hardware1.h_lport.p_vlans, &vlan47, v_entries);
	TAILQ_INSERT_TAIL(&hardware1.h_lport.p_vlans, &vlan49, v_entries);
	TAILQ_INSERT_TAIL(&hardware1.h_lport.p_vlans, &vlan1449, v_entries);
	TAILQ_INIT(&hardware1.h_lport.p_ppvids);
	TAILQ_INSERT_TAIL(&hardware1.h_lport.p_ppvids, &ppvid47, p_entries);
	TAILQ_INSERT_TAIL(&hardware1.h_lport.p_ppvids, &ppvid118, p_entries);
	TAILQ_INIT(&hardware1.h_lport.p_pids);
	TAILQ_INSERT_TAIL(&hardware1.h_lport.p_pids, &pi88cc, p_entries);
	TAILQ_INSERT_TAIL(&hardware1.h_lport.p_pids, &pi888e01, p_entries);
	TAILQ_INIT(&hardware2.h_lport.p_vlans);
	TAILQ_INIT(&hardware2.h_lport.p_ppvids);
	TAILQ_INIT(&hardware2.h_lport.p_pids);
	TAILQ_INIT(&port2.p_vlans);
	TAILQ_INIT(&port2.p_ppvids);
	TAILQ_INIT(&port2.p_pids);
#endif
	TAILQ_INIT(&hardware1.h_rports);
	TAILQ_INSERT_TAIL(&hardware1.h_rports, &port2, p_entries);
	TAILQ_INSERT_TAIL(&hardware1.h_rports, &hardware2.h_lport, p_entries);
	TAILQ_INIT(&hardware2.h_rports);
	TAILQ_INSERT_TAIL(&hardware2.h_rports, &hardware1.h_lport, p_entries);
}

/* Convert OID to a string. Static buffer. */
char*
snmp_oidrepr(oid *name, size_t namelen)
{
	static char *buffer[4] = {NULL, NULL, NULL, NULL};
	static int current = 0;
	size_t i;

	current = (current + 1)%4;
	free(buffer[current]); buffer[current] = NULL;

	for (i = 0; i < namelen; i++) {
		/* Not very efficient... */
		char *newbuffer = NULL;
		if (asprintf(&newbuffer, "%s.%lu", buffer[current]?buffer[current]:"",
			(unsigned long)name[i]) == -1) {
			free(buffer[current]);
			buffer[current] = NULL;
			return NULL;
		}
		free(buffer[current]);
		buffer[current] = newbuffer;
	}
	return buffer[current++];
}

struct tree_node {
	oid    name[MAX_OID_LEN];
	size_t namelen;
	int    type;		/* ASN_* */
	union {
		unsigned long int integer;
		struct {
			char *octet;
			size_t len;
		} string;
	} value;
};

static oid zeroDotZero[2] = {0, 0};
struct tree_node snmp_tree[] = {
	{ {1, 1, 1, 0}, 4, ASN_INTEGER,   { .integer = 30 } }, /* lldpMessageTxInterval */
	{ {1, 1, 2, 0}, 4, ASN_INTEGER,   { .integer = 2 } },  /* lldpMessageTxHoldMultiplier */
	{ {1, 1, 3, 0}, 4, ASN_INTEGER,   { .integer = 1 } },  /* lldpReinitDelay */
	{ {1, 1, 4, 0}, 4, ASN_INTEGER,   { .integer = 1 } },  /* lldpTxDelay */
	{ {1, 1, 5, 0}, 4, ASN_INTEGER,   { .integer = 5 } },  /* lldpNotificationInterval */
	{ {1, 2, 1, 0}, 4, ASN_TIMETICKS, { .integer = 10000 } },/* lldpStatsRemTablesLastChangeTime */
	{ {1, 2, 2, 0}, 4, ASN_GAUGE,     { .integer = 1100 } }, /* lldpStatsRemTablesInserts */
	{ {1, 2, 3, 0}, 4, ASN_GAUGE,     { .integer = 56 } }, /* lldpStatsRemTablesDeletes */
	{ {1, 2, 4, 0}, 4, ASN_GAUGE,     { .integer = 2 } }, /* lldpStatsRemTablesDrops */
	{ {1, 2, 5, 0}, 4, ASN_GAUGE,     { .integer = 230 } }, /* lldpStatsRemTablesAgeouts */

	{ {1, 2, 6, 1, 2, 3}, 6, ASN_COUNTER, { .integer = 1352 } }, /* lldpStatsTxPortFramesTotal.3 */
	{ {1, 2, 6, 1, 2, 4}, 6, ASN_COUNTER, { .integer = 11352 } }, /* lldpStatsTxPortFramesTotal.4 */
	{ {1, 2, 7, 1, 2, 3}, 6, ASN_COUNTER, { .integer = 5 } }, /* lldpStatsRxPortFramesDiscardedTotal.3 */
	{ {1, 2, 7, 1, 2, 4}, 6, ASN_COUNTER, { .integer = 55 } }, /* lldpStatsRxPortFramesDiscardedTotal.4 */
	{ {1, 2, 7, 1, 3, 3}, 6, ASN_COUNTER, { .integer = 5 } }, /* lldpStatsRxPortFramesError.3 */
	{ {1, 2, 7, 1, 3, 4}, 6, ASN_COUNTER, { .integer = 55 } }, /* lldpStatsRxPortFramesError.4 */
	{ {1, 2, 7, 1, 4, 3}, 6, ASN_COUNTER, { .integer = 1458 } }, /* lldpStatsRxPortFramesTotal.3 */
	{ {1, 2, 7, 1, 4, 4}, 6, ASN_COUNTER, { .integer = 11458 } }, /* lldpStatsRxPortFramesTotal.4 */
	{ {1, 2, 7, 1, 5, 3}, 6, ASN_COUNTER, { .integer = 4 } }, /* lldpStatsRxPortTLVsDiscardedTotal.3 */
	{ {1, 2, 7, 1, 5, 4}, 6, ASN_COUNTER, { .integer = 14 } }, /* lldpStatsRxPortTLVsDiscardedTotal.4 */
	{ {1, 2, 7, 1, 6, 3}, 6, ASN_COUNTER, { .integer = 4 } }, /* lldpStatsRxPortTLVsUnrecognizedTotal.3 */
	{ {1, 2, 7, 1, 6, 4}, 6, ASN_COUNTER, { .integer = 14 } }, /* lldpStatsRxPortTLVsUnrecognizedTotal.4 */
	{ {1, 2, 7, 1, 7, 3}, 6, ASN_GAUGE,   { .integer = 20 } }, /* lldpStatsRxPortAgeoutsTotal.3 */
	{ {1, 2, 7, 1, 7, 4}, 6, ASN_GAUGE,   { .integer = 210 } }, /* lldpStatsRxPortAgeoutsTotal.4 */

	{ {1, 3, 1, 0}, 4, ASN_INTEGER,   { .integer = 4 } }, /* lldpLocChassisIdSubtype */
	/* lldpLocChassisId */
	{ {1, 3, 2, 0}, 4, ASN_OCTET_STR, { .string = { .octet = "AAA012",
							.len = 6 } }},
	/* lldpLocSysName */
	{ {1, 3, 3, 0}, 4, ASN_OCTET_STR, { .string = { .octet = "chassis1.example.com",
							.len = 20 } }},
	/* lldpLocSysDesc */
	{ {1, 3, 4, 0}, 4, ASN_OCTET_STR, { .string = { .octet = "First chassis",
							.len = 13 } }},
	/* lldpLocSysCapSupported */
	{ {1, 3, 5, 0}, 4, ASN_OCTET_STR, { .string = { .octet = "\x38",
							.len = 1 } }},
	/* lldpLocSysCapEnabled */
	{ {1, 3, 6, 0}, 4, ASN_OCTET_STR, { .string = { .octet = "\x8",
							.len = 1 } }},

	{ {1, 3, 7, 1, 2, 3}, 6, ASN_INTEGER, { .integer = 3 } }, /* lldpLocPortIdSubtype.3 */
	{ {1, 3, 7, 1, 2, 4}, 6, ASN_INTEGER, { .integer = 5 } }, /* lldpLocPortIdSubtype.5 */
	/* lldpLocPortId.3 */
	{ {1, 3, 7, 1, 3, 3}, 6, ASN_OCTET_STR, { .string = { .octet = "AAA012",
							      .len = 6 } }},
	/* lldpLocPortId.4 */
	{ {1, 3, 7, 1, 3, 4}, 6, ASN_OCTET_STR, { .string = { .octet = "eth4",
							      .len = 4 } }},
	/* lldpLocPortDesc.3 */
	{ {1, 3, 7, 1, 4, 3}, 6, ASN_OCTET_STR, { .string = { .octet = "eth2",
							      .len = 4 } }},
	/* lldpLocPortDesc.4 */
	{ {1, 3, 7, 1, 4, 4}, 6, ASN_OCTET_STR, { .string = { .octet = "Intel 1000 GE",
							      .len = 13 } }},

	{ {1, 3, 8, 1, 3, 1, 4, 192, 0, 2, 15}, 11, ASN_INTEGER, { .integer = 5 } }, /* lldpLocManAddrLen */
	{ {1, 3, 8, 1, 4, 1, 4, 192, 0, 2, 15}, 11, ASN_INTEGER, { .integer = 2 } }, /* lldpLocManAddrIfSubtype */
	{ {1, 3, 8, 1, 5, 1, 4, 192, 0, 2, 15}, 11, ASN_INTEGER, { .integer = 3 } }, /* lldpLocManAddrIfId */
	/* lldpLocManAddrOID */
	{ {1, 3, 8, 1, 6, 1, 4, 192, 0, 2, 15}, 11, ASN_OBJECT_ID,
	  { .string = { .octet = (char *)zeroDotZero,
			.len = sizeof(zeroDotZero) }} },

	/* lldpRemChassisIdSubtype */
	{ {1, 4, 1, 1, 4, 0, 3, 1 }, 8, ASN_INTEGER, { .integer = 4 } }, 
	{ {1, 4, 1, 1, 4, 8000, 3, 4}, 8, ASN_INTEGER, { .integer = 7 } },
	{ {1, 4, 1, 1, 4, 10000, 4, 1}, 8, ASN_INTEGER, { .integer = 4 } },
	/* lldpRemChassisId */
	{ {1, 4, 1, 1, 5, 0, 3, 1 }, 8, ASN_OCTET_STR, { .string = { .octet = "AAA012", .len = 6 }} },
	{ {1, 4, 1, 1, 5, 8000, 3, 4}, 8, ASN_OCTET_STR, { .string =
							   { .octet = "chassis2",
							     .len = 6 }} },
	{ {1, 4, 1, 1, 5, 10000, 4, 1}, 8, ASN_OCTET_STR, { .string = { .octet = "AAA012", .len = 6 }} },
	/* lldpRemPortIdSubtype */
	{ {1, 4, 1, 1, 6, 0, 3, 1 }, 8, ASN_INTEGER, { .integer = 5 } }, 
	{ {1, 4, 1, 1, 6, 8000, 3, 4}, 8, ASN_INTEGER, { .integer = 1 } },
	{ {1, 4, 1, 1, 6, 10000, 4, 1}, 8, ASN_INTEGER, { .integer = 3 } },
	/* lldpRemPortId */
	{ {1, 4, 1, 1, 7, 0, 3, 1 }, 8, ASN_OCTET_STR, { .string = { .octet = "eth4", .len = 4 }} },
	{ {1, 4, 1, 1, 7, 8000, 3, 4}, 8, ASN_OCTET_STR, { .string =
							   { .octet = "Giga1/7", .len = 7 }} },
	{ {1, 4, 1, 1, 7, 10000, 4, 1}, 8, ASN_OCTET_STR, { .string = { .octet = "AAA012", .len = 6 }} },
	/* lldpRemPortDesc */
	{ {1, 4, 1, 1, 8, 0, 3, 1 }, 8, ASN_OCTET_STR,
	  { .string = { .octet = "Intel 1000 GE", .len = 13 }} },
	{ {1, 4, 1, 1, 8, 8000, 3, 4}, 8, ASN_OCTET_STR,
	  { .string = { .octet = "Gigabit Ethernet 1/7", .len = 20 }} },
	{ {1, 4, 1, 1, 8, 10000, 4, 1}, 8, ASN_OCTET_STR,
	  { .string = { .octet = "eth2", .len = 4 }} },
	/* lldpRemSysName */
	{ {1, 4, 1, 1, 9, 0, 3, 1 }, 8, ASN_OCTET_STR,
	  { .string = { .octet = "chassis1.example.com", .len = 20 }} },
	{ {1, 4, 1, 1, 9, 8000, 3, 4}, 8, ASN_OCTET_STR,
	  { .string = { .octet = "chassis2.example.com", .len = 20 }} },
	{ {1, 4, 1, 1, 9, 10000, 4, 1}, 8, ASN_OCTET_STR,
	  { .string = { .octet = "chassis1.example.com", .len = 20 }} },
	/* lldpRemSysDesc */
	{ {1, 4, 1, 1, 10, 0, 3, 1 }, 8, ASN_OCTET_STR,
	  { .string = { .octet = "First chassis", .len = 13 }} },
	{ {1, 4, 1, 1, 10, 8000, 3, 4}, 8, ASN_OCTET_STR,
	  { .string = { .octet = "Second chassis", .len = 14 }} },
	{ {1, 4, 1, 1, 10, 10000, 4, 1}, 8, ASN_OCTET_STR,
	  { .string = { .octet = "First chassis", .len = 13 }} },
	/* lldpRemSysCapSupported */
	{ {1, 4, 1, 1, 11, 0, 3, 1 }, 8, ASN_OCTET_STR,
	  { .string = { .octet = "\x38", .len = 1 }} },
	{ {1, 4, 1, 1, 11, 8000, 3, 4}, 8, ASN_OCTET_STR,
	  { .string = { .octet = "\x8", .len = 1 }} },
	{ {1, 4, 1, 1, 11, 10000, 4, 1}, 8, ASN_OCTET_STR,
	  { .string = { .octet = "\x38", .len = 1 }} },
	/* lldpRemSysCapEnabled */
	{ {1, 4, 1, 1, 12, 0, 3, 1 }, 8, ASN_OCTET_STR,
	  { .string = { .octet = "\x8", .len = 1 }} },
	{ {1, 4, 1, 1, 12, 8000, 3, 4}, 8, ASN_OCTET_STR,
	  { .string = { .octet = "\x8", .len = 1 }} },
	{ {1, 4, 1, 1, 12, 10000, 4, 1}, 8, ASN_OCTET_STR,
	  { .string = { .octet = "\x8", .len = 1 }} },

	/* lldpRemManAddrIfSubtype */
	{ {1, 4, 2, 1, 3, 0, 3, 1, 1, 4, 192, 0, 2, 15 }, 14, ASN_INTEGER, { .integer = 2 } },
	{ {1, 4, 2, 1, 3, 8000, 3, 4, 1, 4, 192, 0, 2, 17 }, 14, ASN_INTEGER, { .integer = 2 } },
	{ {1, 4, 2, 1, 3, 8000, 3, 4, 2, 16,
	   0x20, 0x01, 0x0d, 0xb8, 0xca, 0xfe, 0x00, 0x00,
	   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x17 }, 26, ASN_INTEGER, { .integer = 2 } },
	{ {1, 4, 2, 1, 3, 10000, 4, 1, 1, 4, 192, 0, 2, 15 }, 14, ASN_INTEGER, { .integer = 2 } },
	/* lldpRemManAddrIfId */
	{ {1, 4, 2, 1, 4, 0, 3, 1, 1, 4, 192, 0, 2, 15 }, 14, ASN_INTEGER, { .integer = 3 } },
	{ {1, 4, 2, 1, 4, 8000, 3, 4, 1, 4, 192, 0, 2, 17 }, 14, ASN_INTEGER, { .integer = 5 } },
	{ {1, 4, 2, 1, 4, 8000, 3, 4, 2, 16,
	   0x20, 0x01, 0x0d, 0xb8, 0xca, 0xfe, 0x00, 0x00,
	   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x17 }, 26, ASN_INTEGER, { .integer = 5 } },
	{ {1, 4, 2, 1, 4, 10000, 4, 1, 1, 4, 192, 0, 2, 15 }, 14, ASN_INTEGER, { .integer = 3 } },
	/* lldpRemManAddrOID */
	{ {1, 4, 2, 1, 5, 0, 3, 1, 1, 4, 192, 0, 2, 15 }, 14, ASN_OBJECT_ID,
	  { .string = { .octet = (char *)zeroDotZero,
			.len = sizeof(zeroDotZero) }} },
	{ {1, 4, 2, 1, 5, 8000, 3, 4, 1, 4, 192, 0, 2, 17 }, 14, ASN_OBJECT_ID,
	  { .string = { .octet = (char *)zeroDotZero,
			.len = sizeof(zeroDotZero) }} },
	{ {1, 4, 2, 1, 5, 8000, 3, 4, 2, 16,
	   0x20, 0x01, 0x0d, 0xb8, 0xca, 0xfe, 0x00, 0x00,
	   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x17 }, 26, ASN_OBJECT_ID,
	  { .string = { .octet = (char *)zeroDotZero,
			.len = sizeof(zeroDotZero) }} },
	{ {1, 4, 2, 1, 5, 10000, 4, 1, 1, 4, 192, 0, 2, 15 }, 14, ASN_OBJECT_ID,
	  { .string = { .octet = (char *)zeroDotZero,
			.len = sizeof(zeroDotZero) }} },

#ifdef ENABLE_CUSTOM
	/* lldpRemOrgDefInfo */
	{ {1, 4, 4, 1, 4, 0, 3, 1, 33, 44, 55, 44, 1 }, 13, ASN_OCTET_STR,
	  { .string = { .octet = "OUI content", .len = 11 }} },
	{ {1, 4, 4, 1, 4, 0, 3, 1, 33, 44, 55, 44, 2 }, 13, ASN_OCTET_STR,
	  { .string = { .octet = "More content", .len = 12 }} },
	{ {1, 4, 4, 1, 4, 0, 3, 1, 33, 44, 55, 45, 3 }, 13, ASN_OCTET_STR,
	  { .string = { .octet = "More more content", .len = 17 }} },
	{ {1, 4, 4, 1, 4, 0, 3, 1, 33, 44, 56, 44, 4 }, 13, ASN_OCTET_STR,
	  { .string = { .octet = "Even more content", .len = 17 }} },
	{ {1, 4, 4, 1, 4, 10000, 4, 1, 33, 44, 55, 44, 1 }, 13, ASN_OCTET_STR,
	  { .string = { .octet = "Still more content", .len = 18 }} },
#endif

#ifdef ENABLE_DOT3
	/* lldpXdot3LocPortAutoNegSupported */
	{ {1, 5, 4623, 1, 2, 1, 1, 1, 3 }, 9, ASN_INTEGER, { .integer = 1 }},
	{ {1, 5, 4623, 1, 2, 1, 1, 1, 4 }, 9, ASN_INTEGER, { .integer = 1 }},
	/* lldpXdot3LocPortAutoNegEnabled */
	{ {1, 5, 4623, 1, 2, 1, 1, 2, 3 }, 9, ASN_INTEGER, { .integer = 1 }},
	{ {1, 5, 4623, 1, 2, 1, 1, 2, 4 }, 9, ASN_INTEGER, { .integer = 1 }},
	/* lldpXdot3LocPortAutoNegAdvertisedCap */
	{ {1, 5, 4623, 1, 2, 1, 1, 3, 3 }, 9, ASN_OCTET_STR,
	  { .string = { .octet = "\x0c\x00", .len = 2 }} },
	{ {1, 5, 4623, 1, 2, 1, 1, 3, 4 }, 9, ASN_OCTET_STR,
	  { .string = { .octet = "\x04\x01", .len = 2 }} },
	/* lldpXdot3LocPortOperMauType */
	{ {1, 5, 4623, 1, 2, 1, 1, 4, 3 }, 9, ASN_INTEGER, { .integer = 16 }},
	{ {1, 5, 4623, 1, 2, 1, 1, 4, 4 }, 9, ASN_INTEGER, { .integer = 30 }},

	/* lldpXdot3LocPowerPortClass */
	{ {1, 5, 4623, 1, 2, 2, 1, 1, 3 }, 9, ASN_INTEGER, { .integer = 2 }},
	/* lldpXdot3LocPowerMDISupported */
	{ {1, 5, 4623, 1, 2, 2, 1, 2, 3 }, 9, ASN_INTEGER, { .integer = 1 }},
	/* lldpXdot3LocPowerMDIEnabled */
	{ {1, 5, 4623, 1, 2, 2, 1, 3, 3 }, 9, ASN_INTEGER, { .integer = 1 }},
	/* lldpXdot3LocPowerPairControlable */
	{ {1, 5, 4623, 1, 2, 2, 1, 4, 3 }, 9, ASN_INTEGER, { .integer = 1 }},
	/* lldpXdot3LocPowerPairs */
	{ {1, 5, 4623, 1, 2, 2, 1, 5, 3 }, 9, ASN_INTEGER, { .integer = 2 }},
	/* lldpXdot3LocPowerClass */
	{ {1, 5, 4623, 1, 2, 2, 1, 6, 3 }, 9, ASN_INTEGER, { .integer = 3 }},
	/* As per 802.3at-2009, not sure of the OID... */
	/* lldpXdot3LocPowerType */
	{ {1, 5, 4623, 1, 2, 2, 1, 7, 3 }, 9, ASN_OCTET_STR,
	  { .string = { .octet = "\xC0", .len = 1 } }},
	/* lldpXdot3LocPowerSource */
	{ {1, 5, 4623, 1, 2, 2, 1, 8, 3 }, 9, ASN_OCTET_STR,
	  { .string = { .octet = "\xC0", .len = 1 } }},
	/* lldpXdot3LocPowerPriority */
	{ {1, 5, 4623, 1, 2, 2, 1, 9, 3 }, 9, ASN_INTEGER, { .integer = 1 }},
	/* lldpXdot3LocPDRequestedPowerValue */
	{ {1, 5, 4623, 1, 2, 2, 1, 10, 3 }, 9, ASN_INTEGER, { .integer = 2000 }},
	/* lldpXdot3LocPSEAllocatedPowerValue */
	{ {1, 5, 4623, 1, 2, 2, 1, 11, 3 }, 9, ASN_INTEGER, { .integer = 2500 }},

	/* lldpXdot3LocLinkAggStatus */
	{ {1, 5, 4623, 1, 2, 3, 1, 1, 3 }, 9, ASN_OCTET_STR,
	  { .string = { .octet = "\x00", .len = 1 }} },
	{ {1, 5, 4623, 1, 2, 3, 1, 1, 4 }, 9, ASN_OCTET_STR,
	  { .string = { .octet = "\xC0", .len = 1 }} },
	/* lldpXdot3LocLinkAggPortId */
	{ {1, 5, 4623, 1, 2, 3, 1, 2, 3 }, 9, ASN_INTEGER, { .integer = 0 }},
	{ {1, 5, 4623, 1, 2, 3, 1, 2, 4 }, 9, ASN_INTEGER, { .integer = 3 }},

	/* lldpXdot3LocMaxFrameSize */
	{ {1, 5, 4623, 1, 2, 4, 1, 1, 3 }, 9, ASN_INTEGER, { .integer = 1600 }},
	{ {1, 5, 4623, 1, 2, 4, 1, 1, 4 }, 9, ASN_INTEGER, { .integer = 9000 }},

	/* lldpXdot3RemPortAutoNegSupported */
	{ {1, 5, 4623, 1, 3, 1, 1, 1, 0, 3, 1 }, 11, ASN_INTEGER, { .integer = 1 }},
	{ {1, 5, 4623, 1, 3, 1, 1, 1, 8000, 3, 4 }, 11, ASN_INTEGER, { .integer = 2 }},
	{ {1, 5, 4623, 1, 3, 1, 1, 1, 10000, 4, 1 }, 11, ASN_INTEGER, { .integer = 1 }},
	/* lldpXdot3RemPortAutoNegEnabled */
	{ {1, 5, 4623, 1, 3, 1, 1, 2, 0, 3, 1 }, 11, ASN_INTEGER, { .integer = 1 }},
	{ {1, 5, 4623, 1, 3, 1, 1, 2, 8000, 3, 4 }, 11, ASN_INTEGER, { .integer = 2 }},
	{ {1, 5, 4623, 1, 3, 1, 1, 2, 10000, 4, 1 }, 11, ASN_INTEGER, { .integer = 1 }},
	/* lldpXdot3RemPortAutoNegAdvertisedCap */
	{ {1, 5, 4623, 1, 3, 1, 1, 3, 0, 3, 1 }, 11, ASN_OCTET_STR,
	  { .string = { .octet = "\x04\x01", .len = 2 }} },
	{ {1, 5, 4623, 1, 3, 1, 1, 3, 8000, 3, 4 }, 11, ASN_OCTET_STR,
	  { .string = { .octet = "\x00\x00", .len = 2 }} },
	{ {1, 5, 4623, 1, 3, 1, 1, 3, 10000, 4, 1 }, 11, ASN_OCTET_STR,
	  { .string = { .octet = "\x0c\x00", .len = 2 }} },
	/* lldpXdot3RemPortOperMauType */
	{ {1, 5, 4623, 1, 3, 1, 1, 4, 0, 3, 1 }, 11, ASN_INTEGER, { .integer = 30 }},
	{ {1, 5, 4623, 1, 3, 1, 1, 4, 8000, 3, 4 }, 11, ASN_INTEGER, { .integer = 0 }},
	{ {1, 5, 4623, 1, 3, 1, 1, 4, 10000, 4, 1 }, 11, ASN_INTEGER, { .integer = 16 }},

	/* lldpXdot3RemPowerPortClass */
	{ {1, 5, 4623, 1, 3, 2, 1, 1, 10000, 4, 1 }, 11, ASN_INTEGER, { .integer = 2 }},
	/* lldpXdot3RemPowerMDISupported */
	{ {1, 5, 4623, 1, 3, 2, 1, 2, 10000, 4, 1 }, 11, ASN_INTEGER, { .integer = 1 }},
	/* lldpXdot3RemPowerMDIEnabled */
	{ {1, 5, 4623, 1, 3, 2, 1, 3, 10000, 4, 1 }, 11, ASN_INTEGER, { .integer = 1 }},
	/* lldpXdot3RemPowerPairControlable */
	{ {1, 5, 4623, 1, 3, 2, 1, 4, 10000, 4, 1 }, 11, ASN_INTEGER, { .integer = 1 }},
	/* lldpXdot3RemPowerPairs */
	{ {1, 5, 4623, 1, 3, 2, 1, 5, 10000, 4, 1 }, 11, ASN_INTEGER, { .integer = 2 }},
	/* lldpXdot3RemPowerClass */
	{ {1, 5, 4623, 1, 3, 2, 1, 6, 10000, 4, 1 }, 11, ASN_INTEGER, { .integer = 3 }},
	/* As per 802.3at-2009, not sure of the OID... */
	/* lldpXdot3RemPowerType */
	{ {1, 5, 4623, 1, 3, 2, 1, 7, 10000, 4, 1 }, 11, ASN_OCTET_STR,
	  { .string = { .octet = "\xC0", .len = 1 } }},
	/* lldpXdot3RemPowerSource */
	{ {1, 5, 4623, 1, 3, 2, 1, 8, 10000, 4, 1 }, 11, ASN_OCTET_STR,
	  { .string = { .octet = "\xC0", .len = 1 } }},
	/* lldpXdot3RemPowerPriority */
	{ {1, 5, 4623, 1, 3, 2, 1, 9, 10000, 4, 1 }, 11, ASN_INTEGER, { .integer = 1 }},
	/* lldpXdot3RemPDRequestedPowerValue */
	{ {1, 5, 4623, 1, 3, 2, 1, 10, 10000, 4, 1 }, 11, ASN_INTEGER, { .integer = 2000 }},
	/* lldpXdot3RemPSEAllocatedPowerValue */
	{ {1, 5, 4623, 1, 3, 2, 1, 11, 10000, 4, 1 }, 11, ASN_INTEGER, { .integer = 2500 }},

	/* lldpXdot3RemLinkAggStatus */
	{ {1, 5, 4623, 1, 3, 3, 1, 1, 0, 3, 1 }, 11, ASN_OCTET_STR,
	  { .string = { .octet = "\xC0", .len = 1 }} },
	{ {1, 5, 4623, 1, 3, 3, 1, 1, 8000, 3, 4 }, 11, ASN_OCTET_STR,
	  { .string = { .octet = "\x00", .len = 1 }} },
	{ {1, 5, 4623, 1, 3, 3, 1, 1, 10000, 4, 1 }, 11, ASN_OCTET_STR,
	  { .string = { .octet = "\x00", .len = 1 }} },
	/* lldpXdot3RemLinkAggPortId */
	{ {1, 5, 4623, 1, 3, 3, 1, 2, 0, 3, 1 }, 11, ASN_INTEGER, { .integer = 3 }},
	{ {1, 5, 4623, 1, 3, 3, 1, 2, 8000, 3, 4 }, 11, ASN_INTEGER, { .integer = 0 }},
	{ {1, 5, 4623, 1, 3, 3, 1, 2, 10000, 4, 1 }, 11, ASN_INTEGER, { .integer = 0 }},

	/* lldpXdot3RemMaxFrameSize */
	{ {1, 5, 4623, 1, 3, 4, 1, 1, 0, 3, 1 }, 11, ASN_INTEGER, { .integer = 9000 }},
	{ {1, 5, 4623, 1, 3, 4, 1, 1, 10000, 4, 1 }, 11, ASN_INTEGER, { .integer = 1600 }},
#endif
#ifdef ENABLE_LLDPMED
	/* lldpXMedLocDeviceClass */
	{ {1, 5, 4795, 1, 1, 1, 0 }, 7, ASN_INTEGER, { .integer = 2 }},

	/* lldpXMedLocMediaPolicyVlanID */
	{ {1, 5, 4795, 1, 2, 1, 1, 2, 3, 3 }, 10, ASN_INTEGER, { .integer = 475 }},
	{ {1, 5, 4795, 1, 2, 1, 1, 2, 3, 7 }, 10, ASN_INTEGER, { .integer = 472 }},
	{ {1, 5, 4795, 1, 2, 1, 1, 2, 4, 3 }, 10, ASN_INTEGER, { .integer = 475 }},
	{ {1, 5, 4795, 1, 2, 1, 1, 2, 4, 6 }, 10, ASN_INTEGER, { .integer = 1007 }},
	/* lldpXMedLocMediaPolicyPriority */
	{ {1, 5, 4795, 1, 2, 1, 1, 3, 3, 3 }, 10, ASN_INTEGER, { .integer = 3 }},
	{ {1, 5, 4795, 1, 2, 1, 1, 3, 3, 7 }, 10, ASN_INTEGER, { .integer = 1 }},
	{ {1, 5, 4795, 1, 2, 1, 1, 3, 4, 3 }, 10, ASN_INTEGER, { .integer = 3 }},
	{ {1, 5, 4795, 1, 2, 1, 1, 3, 4, 6 }, 10, ASN_INTEGER, { .integer = 1 }},
	/* lldpXMedLocMediaPolicyDscp */
	{ {1, 5, 4795, 1, 2, 1, 1, 4, 3, 3 }, 10, ASN_INTEGER, { .integer = 62 }},
	{ {1, 5, 4795, 1, 2, 1, 1, 4, 3, 7 }, 10, ASN_INTEGER, { .integer = 60 }},
	{ {1, 5, 4795, 1, 2, 1, 1, 4, 4, 3 }, 10, ASN_INTEGER, { .integer = 62 }},
	{ {1, 5, 4795, 1, 2, 1, 1, 4, 4, 6 }, 10, ASN_INTEGER, { .integer = 49 }},
	/* lldpXMedLocMediaPolicyUnknown */
	{ {1, 5, 4795, 1, 2, 1, 1, 5, 3, 3 }, 10, ASN_INTEGER, { .integer = 1 }},
	{ {1, 5, 4795, 1, 2, 1, 1, 5, 3, 7 }, 10, ASN_INTEGER, { .integer = 2 }},
	{ {1, 5, 4795, 1, 2, 1, 1, 5, 4, 3 }, 10, ASN_INTEGER, { .integer = 1 }},
	{ {1, 5, 4795, 1, 2, 1, 1, 5, 4, 6 }, 10, ASN_INTEGER, { .integer = 2 }},
	/* lldpXMedLocMediaPolicyTagged */
	{ {1, 5, 4795, 1, 2, 1, 1, 6, 3, 3 }, 10, ASN_INTEGER, { .integer = 1 }},
	{ {1, 5, 4795, 1, 2, 1, 1, 6, 3, 7 }, 10, ASN_INTEGER, { .integer = 1 }},
	{ {1, 5, 4795, 1, 2, 1, 1, 6, 4, 3 }, 10, ASN_INTEGER, { .integer = 1 }},
	{ {1, 5, 4795, 1, 2, 1, 1, 6, 4, 6 }, 10, ASN_INTEGER, { .integer = 2 }},

	/* lldpXMedLocHardwareRev */
	{ {1, 5, 4795, 1, 2, 2, 0 }, 7, ASN_OCTET_STR,
	  { .string = { .octet = "Hardware 1", .len = 10 }} },
	/* lldpXMedLocSoftwareRev */
	{ {1, 5, 4795, 1, 2, 4, 0 }, 7, ASN_OCTET_STR,
	  { .string = { .octet = "Software 1", .len = 10 }} },
	/* lldpXMedLocSerialNum */
	{ {1, 5, 4795, 1, 2, 5, 0 }, 7, ASN_OCTET_STR,
	  { .string = { .octet = "00-00-0000-AAAA", .len = 15 }} },
	/* lldpXMedLocMfgName */
	{ {1, 5, 4795, 1, 2, 6, 0 }, 7, ASN_OCTET_STR,
	  { .string = { .octet = "Manufacturer 1", .len = 14 }} },
	/* lldpXMedLocModelName */
	{ {1, 5, 4795, 1, 2, 7, 0 }, 7, ASN_OCTET_STR,
	  { .string = { .octet = "Model 1", .len = 7 }} },
	/* lldpXMedLocAssetID */
	{ {1, 5, 4795, 1, 2, 8, 0 }, 7, ASN_OCTET_STR,
	  { .string = { .octet = "Asset 1", .len = 7 }} },

	/* lldpXMedLocLocationInfo */
	{ {1, 5, 4795, 1, 2, 9, 1, 2, 3, 3}, 10, ASN_OCTET_STR,
	  { .string = { .octet = "\x15" "\x02" "FR" "\x06" "\x0d" "Commercial Rd" "\x13" "\x01" "4", .len = 22 }} },
	{ {1, 5, 4795, 1, 2, 9, 1, 2, 4, 2}, 10, ASN_OCTET_STR,
	  { .string = { .octet = "Not interpreted", .len = 15 }} },

	/* lldpXMedLocXPoEDeviceType */
	{ {1, 5, 4795, 1, 2, 10, 0 }, 7, ASN_INTEGER, { .integer = 3 }},
	/* lldpXMedLocXPoEPDPowerReq */
	{ {1, 5, 4795, 1, 2, 13, 0 }, 7, ASN_GAUGE, { .integer = 100 }},
	/* lldpXMedLocXPoEPDPowerSource */
	{ {1, 5, 4795, 1, 2, 14, 0 }, 7, ASN_INTEGER, { .integer = 3 }},
	/* lldpXMedLocXPoEPDPowerPriority */
	{ {1, 5, 4795, 1, 2, 15, 0 }, 7, ASN_INTEGER, { .integer = 3 }},

	/* lldpXMedRemCapSupported */
	{ {1, 5, 4795, 1, 3, 1, 1, 1, 0, 3, 1 }, 11, ASN_OCTET_STR,
	  { .string = { .octet = "\xFC", .len = 1 }} },
	{ {1, 5, 4795, 1, 3, 1, 1, 1, 10000, 4, 1 }, 11, ASN_OCTET_STR,
	  { .string = { .octet = "\xFC", .len = 1 }}},
	/* lldpXMedRemCapCurrent */
	{ {1, 5, 4795, 1, 3, 1, 1, 2, 0, 3, 1 }, 11, ASN_OCTET_STR,
	  { .string = { .octet = "\xFC", .len = 1 }} },
	{ {1, 5, 4795, 1, 3, 1, 1, 2, 10000, 4, 1 }, 11, ASN_OCTET_STR,
	  { .string = { .octet = "\xEC", .len = 1 }}},
	/* lldpXMedRemDeviceClass */
	{ {1, 5, 4795, 1, 3, 1, 1, 3, 0, 3, 1 }, 11, ASN_INTEGER, { .integer = 2 }},
	{ {1, 5, 4795, 1, 3, 1, 1, 3, 10000, 4, 1 }, 11, ASN_INTEGER, { .integer = 2 }},

	/* lldpXMedRemMediaPolicyVlanID */
	{ {1, 5, 4795, 1, 3, 2, 1, 2, 0, 3, 1, 3 }, 12, ASN_INTEGER, { .integer = 475 }},
	{ {1, 5, 4795, 1, 3, 2, 1, 2, 0, 3, 1, 6 }, 12, ASN_INTEGER, { .integer = 1007 }},
	{ {1, 5, 4795, 1, 3, 2, 1, 2, 10000, 4, 1, 3 }, 12, ASN_INTEGER, { .integer = 475 }},
	{ {1, 5, 4795, 1, 3, 2, 1, 2, 10000, 4, 1, 7 }, 12, ASN_INTEGER, { .integer = 472 }},
	/* lldpXMedRemMediaPolicyPriority */
	{ {1, 5, 4795, 1, 3, 2, 1, 3, 0, 3, 1, 3 }, 12, ASN_INTEGER, { .integer = 3 }},
	{ {1, 5, 4795, 1, 3, 2, 1, 3, 0, 3, 1, 6 }, 12, ASN_INTEGER, { .integer = 1 }},
	{ {1, 5, 4795, 1, 3, 2, 1, 3, 10000, 4, 1, 3 }, 12, ASN_INTEGER, { .integer = 3 }},
	{ {1, 5, 4795, 1, 3, 2, 1, 3, 10000, 4, 1, 7 }, 12, ASN_INTEGER, { .integer = 1 }},
	/* lldpXMedLocMediaPolicyDscp */
	{ {1, 5, 4795, 1, 3, 2, 1, 4, 0, 3, 1, 3 }, 12, ASN_INTEGER, { .integer = 62 }},
	{ {1, 5, 4795, 1, 3, 2, 1, 4, 0, 3, 1, 6 }, 12, ASN_INTEGER, { .integer = 49 }},
	{ {1, 5, 4795, 1, 3, 2, 1, 4, 10000, 4, 1, 3 }, 12, ASN_INTEGER, { .integer = 62 }},
	{ {1, 5, 4795, 1, 3, 2, 1, 4, 10000, 4, 1, 7 }, 12, ASN_INTEGER, { .integer = 60 }},
	/* lldpXMedLocMediaPolicyUnknown */
	{ {1, 5, 4795, 1, 3, 2, 1, 5, 0, 3, 1, 3 }, 12, ASN_INTEGER, { .integer = 1 }},
	{ {1, 5, 4795, 1, 3, 2, 1, 5, 0, 3, 1, 6 }, 12, ASN_INTEGER, { .integer = 2 }},
	{ {1, 5, 4795, 1, 3, 2, 1, 5, 10000, 4, 1, 3 }, 12, ASN_INTEGER, { .integer = 1 }},
	{ {1, 5, 4795, 1, 3, 2, 1, 5, 10000, 4, 1, 7 }, 12, ASN_INTEGER, { .integer = 2 }},
	/* lldpXMedLocMediaPolicyTagged */
	{ {1, 5, 4795, 1, 3, 2, 1, 6, 0, 3, 1, 3 }, 12, ASN_INTEGER, { .integer = 1 }},
	{ {1, 5, 4795, 1, 3, 2, 1, 6, 0, 3, 1, 6 }, 12, ASN_INTEGER, { .integer = 2 }},
	{ {1, 5, 4795, 1, 3, 2, 1, 6, 10000, 4, 1, 3 }, 12, ASN_INTEGER, { .integer = 1 }},
	{ {1, 5, 4795, 1, 3, 2, 1, 6, 10000, 4, 1, 7 }, 12, ASN_INTEGER, { .integer = 1 }},

	/* lldpXMedRemHardwareRev */
	{ {1, 5, 4795, 1, 3, 3, 1, 1, 0, 3, 1 }, 11, ASN_OCTET_STR,
	  { .string = { .octet = "Hardware 1", .len = 10 }} },
	{ {1, 5, 4795, 1, 3, 3, 1, 1, 10000, 4, 1 }, 11, ASN_OCTET_STR,
	  { .string = { .octet = "Hardware 1", .len = 10 }} },
	/* lldpXMedRemSoftwareRev */
	{ {1, 5, 4795, 1, 3, 3, 1, 3, 0, 3, 1 }, 11, ASN_OCTET_STR,
	  { .string = { .octet = "Software 1", .len = 10 }} },
	{ {1, 5, 4795, 1, 3, 3, 1, 3, 10000, 4, 1 }, 11, ASN_OCTET_STR,
	  { .string = { .octet = "Software 1", .len = 10 }} },
	/* lldpXMedRemSerialNum */
	{ {1, 5, 4795, 1, 3, 3, 1, 4, 0, 3, 1 }, 11, ASN_OCTET_STR,
	  { .string = { .octet = "00-00-0000-AAAA", .len = 15 }} },
	{ {1, 5, 4795, 1, 3, 3, 1, 4, 10000, 4, 1 }, 11, ASN_OCTET_STR,
	  { .string = { .octet = "00-00-0000-AAAA", .len = 15 }} },
	/* lldpXMedRemMfgName */
	{ {1, 5, 4795, 1, 3, 3, 1, 5, 0, 3, 1 }, 11, ASN_OCTET_STR,
	  { .string = { .octet = "Manufacturer 1", .len = 14 }} },
	{ {1, 5, 4795, 1, 3, 3, 1, 5, 10000, 4, 1 }, 11, ASN_OCTET_STR,
	  { .string = { .octet = "Manufacturer 1", .len = 14 }} },
	/* lldpXMedRemModelName */
	{ {1, 5, 4795, 1, 3, 3, 1, 6, 0, 3, 1 }, 11, ASN_OCTET_STR,
	  { .string = { .octet = "Model 1", .len = 7 }} },
	{ {1, 5, 4795, 1, 3, 3, 1, 6, 10000, 4, 1 }, 11, ASN_OCTET_STR,
	  { .string = { .octet = "Model 1", .len = 7 }} },
	/* lldpXMedRemAssetID */
	{ {1, 5, 4795, 1, 3, 3, 1, 7, 0, 3, 1 }, 11, ASN_OCTET_STR,
	  { .string = { .octet = "Asset 1", .len = 7 }} },
	{ {1, 5, 4795, 1, 3, 3, 1, 7, 10000, 4, 1 }, 11, ASN_OCTET_STR,
	  { .string = { .octet = "Asset 1", .len = 7 }} },

	/* lldpXMedLocLocationInfo */
	{ {1, 5, 4795, 1, 3, 4, 1, 2, 0, 3, 1, 2}, 12, ASN_OCTET_STR,
	  { .string = { .octet = "Not interpreted", .len = 15 }} },
	{ {1, 5, 4795, 1, 3, 4, 1, 2, 10000, 4, 1, 3}, 12, ASN_OCTET_STR,
	  { .string = { .octet = "\x15" "\x02" "FR" "\x06" "\x0d" "Commercial Rd" "\x13" "\x01" "4", .len = 22 }} },

	/* lldpXMedRemXPoEDeviceType */
	{ {1, 5, 4795, 1, 3, 5, 1, 1, 0, 3, 1}, 11, ASN_INTEGER, { .integer = 4 }},
	{ {1, 5, 4795, 1, 3, 5, 1, 1, 10000, 4, 1}, 11, ASN_INTEGER, { .integer = 3 }},

	/* lldpXMedRemXPoEPDPowerReq */
	{ {1, 5, 4795, 1, 3, 7, 1, 1, 10000, 4, 1}, 11, ASN_GAUGE, { .integer = 100 }},
	/* lldpXMedRemXPoEPDPowerSource */
	{ {1, 5, 4795, 1, 3, 7, 1, 2, 10000, 4, 1}, 11, ASN_INTEGER, { .integer = 3 }},
	/* lldpXMedRemXPoEPDPowerPriority */
	{ {1, 5, 4795, 1, 3, 7, 1, 3, 10000, 4, 1}, 11, ASN_INTEGER, { .integer = 3 }},

#endif
#ifdef ENABLE_DOT1
	/* lldpXdot1LocPortVlanId */
	{ { 1, 5, 32962, 1, 2, 1, 1, 1, 3}, 9, ASN_INTEGER, { .integer = 47 }},
	{ { 1, 5, 32962, 1, 2, 1, 1, 1, 4}, 9, ASN_INTEGER, { .integer = 0 }},
	/* lldpXdot1LocProtoVlanSupported */
	{ { 1, 5, 32962, 1, 2, 2, 1, 2, 3, 47}, 10, ASN_INTEGER, { .integer = 1 }},
	{ { 1, 5, 32962, 1, 2, 2, 1, 2, 3, 118}, 10, ASN_INTEGER, { .integer = 1 }},
	/* lldpXdot1LocProtoVlanEnabled */
	{ { 1, 5, 32962, 1, 2, 2, 1, 3, 3, 47}, 10, ASN_INTEGER, { .integer = 1 }},
	{ { 1, 5, 32962, 1, 2, 2, 1, 3, 3, 118}, 10, ASN_INTEGER, { .integer = 1 }},
	/* lldpXdot1LocVlanName */
	{ { 1, 5, 32962, 1, 2, 3, 1, 2, 3, 47}, 10, ASN_OCTET_STR,
	  { .string = { .octet = "VLAN #47", .len = 8 }} },
	{ { 1, 5, 32962, 1, 2, 3, 1, 2, 3, 49}, 10, ASN_OCTET_STR,
	  { .string = { .octet = "VLAN #49", .len = 8 }} },
	{ { 1, 5, 32962, 1, 2, 3, 1, 2, 3, 1449}, 10, ASN_OCTET_STR,
	  { .string = { .octet = "VLAN #1449", .len = 10 }} },
	/* lldpXdot1LocProtocolId */
	{ { 1, 5, 32962, 1, 2, 4, 1, 2, 3, 30321}, 10, ASN_OCTET_STR,
	  { .string = { .octet = "\x88\x8e\x01", .len = 3 } }},
	{ { 1, 5, 32962, 1, 2, 4, 1, 2, 3, 30515}, 10, ASN_OCTET_STR,
	  { .string = { .octet = "\x88\xcc", .len = 2 } }},

	/* lldpXdot1RemPortVlanId */
	{ { 1, 5, 32962, 1, 3, 1, 1, 1, 0, 3, 1}, 11, ASN_INTEGER, { .integer = 0 }},
	{ { 1, 5, 32962, 1, 3, 1, 1, 1, 8000, 3, 4}, 11, ASN_INTEGER, { .integer = 0 }},
	{ { 1, 5, 32962, 1, 3, 1, 1, 1, 10000, 4, 1}, 11, ASN_INTEGER, { .integer = 47 }},
	/* lldpXdot1RemProtoVlanSupported */
	{ { 1, 5, 32962, 1, 3, 2, 1, 2, 10000, 4, 1, 47}, 12, ASN_INTEGER, { .integer = 1 }},
	{ { 1, 5, 32962, 1, 3, 2, 1, 2, 10000, 4, 1, 118}, 12, ASN_INTEGER, { .integer = 1 }},
	/* lldpXdot1RemProtoVlanEnabled */
	{ { 1, 5, 32962, 1, 3, 2, 1, 3, 10000, 4, 1, 47}, 12, ASN_INTEGER, { .integer = 1 }},
	{ { 1, 5, 32962, 1, 3, 2, 1, 3, 10000, 4, 1, 118}, 12, ASN_INTEGER, { .integer = 1 }},
	/* lldpXdot1RemVlanName */
	{ { 1, 5, 32962, 1, 3, 3, 1, 2, 10000, 4, 1, 47}, 12, ASN_OCTET_STR,
	  { .string = { .octet = "VLAN #47", .len = 8 }} },
	{ { 1, 5, 32962, 1, 3, 3, 1, 2, 10000, 4, 1, 49}, 12, ASN_OCTET_STR,
	  { .string = { .octet = "VLAN #49", .len = 8 }} },
	{ { 1, 5, 32962, 1, 3, 3, 1, 2, 10000, 4, 1, 1449}, 12, ASN_OCTET_STR,
	  { .string = { .octet = "VLAN #1449", .len = 10 }} },
	/* lldpXdot1RemProtocolId */
	{ { 1, 5, 32962, 1, 3, 4, 1, 2, 10000, 4, 1, 30321}, 12, ASN_OCTET_STR,
	  { .string = { .octet = "\x88\x8e\x01", .len = 3 } }},
	{ { 1, 5, 32962, 1, 3, 4, 1, 2, 10000, 4, 1, 30515}, 12, ASN_OCTET_STR,
	  { .string = { .octet = "\x88\xcc", .len = 2 } }}
#endif
};

char*
tohex(char *str, size_t len)
{
	static char *hex[] = { NULL, NULL };
	static int which = 0;
	free(hex[which]); hex[which] = NULL;
	hex[which] = malloc(len * 3 + 1);
	fail_unless(hex[which] != NULL, "Not enough memory?");
	for (size_t i = 0; i < len; i++)
		snprintf(hex[which] + 3*i, 4, "%02X ", (unsigned char)str[i]);
	which = 1 - which;
	return hex[1 - which];
}

int
snmp_is_prefix_of(struct variable8 *vp, struct tree_node *n, char *repr)
{
	if (n->namelen < vp->namelen) return 0;
	if (memcmp(n->name,
		   vp->name,
		   vp->namelen * sizeof(oid)))
		return 0;
	fail_unless(n->type == vp->type, "Inappropriate type for OID %s", repr);
	return 1;
}

void
snmp_merge(struct variable8 *v1, struct tree_node *n, struct variable *vp,
	   oid *target, size_t *targetlen)
{
	vp->magic = v1->magic;
	vp->type = v1->type;
	vp->acl = v1->acl;
	vp->findVar = v1->findVar;
	vp->namelen = v1->namelen +
		sizeof(lldp_oid)/sizeof(oid);
	memcpy(vp->name, lldp_oid, sizeof(lldp_oid));
	memcpy(vp->name + sizeof(lldp_oid)/sizeof(oid),
	       v1->name,
	       v1->namelen*sizeof(oid));
	*targetlen = n->namelen +
		sizeof(lldp_oid)/sizeof(oid);
	memcpy(target, lldp_oid, sizeof(lldp_oid));
	memcpy(target + sizeof(lldp_oid)/sizeof(oid),
	       n->name,
	       n->namelen * sizeof(oid));
}

void
snmp_compare(struct tree_node *n,
	     u_char *result, size_t varlen,
	     oid *target, size_t targetlen, char *repr)
{
	unsigned long int value;
	fail_unless((targetlen == sizeof(lldp_oid)/sizeof(oid) + n->namelen) &&
		    !memcmp(target, lldp_oid, sizeof(lldp_oid)) &&
		    !memcmp(target + sizeof(lldp_oid)/sizeof(oid),
			    n->name,
			    n->namelen * sizeof(oid)),
		    "Bad OID returned when querying %s: got %s", repr,
		    snmp_oidrepr(target, targetlen));
	switch (n->type) {
	case ASN_INTEGER:
	case ASN_TIMETICKS:
	case ASN_GAUGE:
	case ASN_COUNTER:
		fail_unless(varlen == sizeof(unsigned long int),
			    "Inappropriate length for integer type for OID %s",
			    repr);
		memcpy(&value, result, sizeof(value));
		fail_unless(n->value.integer == value,
			    "For OID %s, expected value %u but got %u instead",
			    repr,
			    n->value.integer,
			    value);
		break;
	default:
		fail_unless(((n->value.string.len == varlen) &&
			     !memcmp(n->value.string.octet,
				    result, varlen)),
			    "OID %s: wanted %s, got %s",
			    repr,
			    tohex(n->value.string.octet, n->value.string.len),
			    tohex((char *)result, varlen));
	}
}

START_TEST (test_variable_order)
{
	size_t i;
	for (i = 0; i < agent_lldp_vars_size() - 1; i++) {
		fail_unless(snmp_oid_compare(agent_lldp_vars[i].name,
					     agent_lldp_vars[i].namelen,
					     agent_lldp_vars[i+1].name,
					     agent_lldp_vars[i+1].namelen) < 0,
			    "Registered OID are out of orders (see %s and next one)",
			    snmp_oidrepr(agent_lldp_vars[i].name,
					 agent_lldp_vars[i].namelen));
	}
}
END_TEST

START_TEST (test_get)
{
	size_t j;
	for (j = 0;
	     j < sizeof(snmp_tree)/sizeof(struct tree_node);
	     j++) {
		size_t          i;
		int             found = 0;
		struct variable vp;
		oid             target[MAX_OID_LEN];
		size_t          targetlen;
		size_t          varlen;
		u_char         *result;
		WriteMethod    *wmethod;
		char           *repr = snmp_oidrepr(snmp_tree[j].name,
						    snmp_tree[j].namelen);

		for (i = 0; i < agent_lldp_vars_size(); i++) {
			/* Search for the appropriate prefix. */
			if (!snmp_is_prefix_of(&agent_lldp_vars[i], &snmp_tree[j],
					       repr)) continue;

			/* We have our prefix. Fill out the vp struct
			   correctly. We need to complete OID with
			   LLDP prefix. */
			snmp_merge(&agent_lldp_vars[i], &snmp_tree[j], &vp, target, &targetlen);

			/* Invoke the function */
			result = vp.findVar(&vp, target, &targetlen, 1, &varlen, &wmethod);

			/* Check the result */
			fail_unless(result != NULL,
				    "No result when querying %s", repr);
			snmp_compare(&snmp_tree[j], result, varlen, target, targetlen, repr);

			found = 1;
			break;
		}
		if (!found)
			fail("OID %s not found", repr);
	}
}
END_TEST

START_TEST (test_getnext)
{
	size_t j;
	size_t end = sizeof(snmp_tree)/sizeof(struct tree_node);
	for (j = 0;
	     j < end;
	     j++) {
		size_t          i;
		struct variable vp;
		oid             target[MAX_OID_LEN];
		size_t          targetlen;
		size_t          varlen;
		u_char         *result = NULL;
		WriteMethod    *wmethod;
		char           *repr = snmp_oidrepr(snmp_tree[j].name,
						    snmp_tree[j].namelen);
		for (i = 0; i < agent_lldp_vars_size(); i++) {
			snmp_merge(&agent_lldp_vars[i], &snmp_tree[j], &vp, target, &targetlen);
			result = vp.findVar(&vp, target, &targetlen, 0, &varlen, &wmethod);
			if (result) /* Check next! */
				break;
		}
		if (!result) {
			fail_unless(j == end - 1,
				    "No next result found for %s", repr);
			return;
		}
		fail_unless(j < end - 1,
			    "More results after %s", repr);

		/* For unknown reasons, snmp_compare can be executed
		   even when the above test fails... */
		if (j < end - 1)
			snmp_compare(&snmp_tree[j+1], result, varlen, target, targetlen, repr);
		
	}	
}
END_TEST

Suite *
snmp_suite(void)
{
	Suite *s = suite_create("SNMP");

	TCase *tc_snmp = tcase_create("SNMP");
	tcase_add_checked_fixture(tc_snmp, snmp_config, NULL);
	tcase_add_test(tc_snmp, test_variable_order);
	tcase_add_test(tc_snmp, test_get);
	tcase_add_test(tc_snmp, test_getnext);
	suite_add_tcase(s, tc_snmp);

	return s;
}

int
main()
{
	int number_failed;
	Suite *s = snmp_suite();
	SRunner *sr = srunner_create(s);
	srunner_run_all(sr, CK_ENV);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
