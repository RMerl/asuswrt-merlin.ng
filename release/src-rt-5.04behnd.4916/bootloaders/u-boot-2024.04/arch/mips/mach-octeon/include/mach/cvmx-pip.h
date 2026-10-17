/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * Interface to the hardware Packet Input Processing unit.
 */

#ifndef __CVMX_PIP_H__
#define __CVMX_PIP_H__

#include "cvmx-wqe.h"
#include "cvmx-pki.h"
#include "cvmx-helper-pki.h"

#include "cvmx-helper.h"
#include "cvmx-helper-util.h"
#include "cvmx-pki-resources.h"

#define CVMX_PIP_NUM_INPUT_PORTS 46
#define CVMX_PIP_NUM_WATCHERS	 8

/*
 * Encodes the different error and exception codes
 */
typedef enum {
	CVMX_PIP_L4_NO_ERR = 0ull,
	/*        1  = TCP (UDP) packet not long enough to cover TCP (UDP) header */
	CVMX_PIP_L4_MAL_ERR = 1ull,
	/*        2  = TCP/UDP checksum failure */
	CVMX_PIP_CHK_ERR = 2ull,
	/*        3  = TCP/UDP length check (TCP/UDP length does not match IP length) */
	CVMX_PIP_L4_LENGTH_ERR = 3ull,
	/*        4  = illegal TCP/UDP port (either source or dest port is zero) */
	CVMX_PIP_BAD_PRT_ERR = 4ull,
	/*        8  = TCP flags = FIN only */
	CVMX_PIP_TCP_FLG8_ERR = 8ull,
	/*        9  = TCP flags = 0 */
	CVMX_PIP_TCP_FLG9_ERR = 9ull,
	/*        10 = TCP flags = FIN+RST+* */
	CVMX_PIP_TCP_FLG10_ERR = 10ull,
	/*        11 = TCP flags = SYN+URG+* */
	CVMX_PIP_TCP_FLG11_ERR = 11ull,
	/*        12 = TCP flags = SYN+RST+* */
	CVMX_PIP_TCP_FLG12_ERR = 12ull,
	/*        13 = TCP flags = SYN+FIN+* */
	CVMX_PIP_TCP_FLG13_ERR = 13ull
} cvmx_pip_l4_err_t;

typedef enum {
	CVMX_PIP_IP_NO_ERR = 0ull,
	/*        1 = not IPv4 or IPv6 */
	CVMX_PIP_NOT_IP = 1ull,
	/*        2 = IPv4 header checksum violation */
	CVMX_PIP_IPV4_HDR_CHK = 2ull,
	/*        3 = malformed (packet not long enough to cover IP hdr) */
	CVMX_PIP_IP_MAL_HDR = 3ull,
	/*        4 = malformed (packet not long enough to cover len in IP hdr) */
	CVMX_PIP_IP_MAL_PKT = 4ull,
	/*        5 = TTL / hop count equal zero */
	CVMX_PIP_TTL_HOP = 5ull,
	/*        6 = IPv4 options / IPv6 early extension headers */
	CVMX_PIP_OPTS = 6ull
} cvmx_pip_ip_exc_t;

/**
 * NOTES
 *       late collision (data received before collision)
 *            late collisions cannot be detected by the receiver
 *            they would appear as JAM bits which would appear as bad FCS
 *            or carrier extend error which is CVMX_PIP_EXTEND_ERR
 */
typedef enum {
	/**
	 * No error
	 */
	CVMX_PIP_RX_NO_ERR = 0ull,

	CVMX_PIP_PARTIAL_ERR =
		1ull, /* RGM+SPI            1 = partially received packet (buffering/bandwidth not adequate) */
	CVMX_PIP_JABBER_ERR =
		2ull, /* RGM+SPI            2 = receive packet too large and truncated */
	CVMX_PIP_OVER_FCS_ERR =
		3ull, /* RGM                3 = max frame error (pkt len > max frame len) (with FCS error) */
	CVMX_PIP_OVER_ERR =
		4ull, /* RGM+SPI            4 = max frame error (pkt len > max frame len) */
	CVMX_PIP_ALIGN_ERR =
		5ull, /* RGM                5 = nibble error (data not byte multiple - 100M and 10M only) */
	CVMX_PIP_UNDER_FCS_ERR =
		6ull, /* RGM                6 = min frame error (pkt len < min frame len) (with FCS error) */
	CVMX_PIP_GMX_FCS_ERR = 7ull, /* RGM                7 = FCS error */
	CVMX_PIP_UNDER_ERR =
		8ull, /* RGM+SPI            8 = min frame error (pkt len < min frame len) */
	CVMX_PIP_EXTEND_ERR = 9ull, /* RGM                9 = Frame carrier extend error */
	CVMX_PIP_TERMINATE_ERR =
		9ull, /* XAUI               9 = Packet was terminated with an idle cycle */
	CVMX_PIP_LENGTH_ERR =
		10ull, /* RGM               10 = length mismatch (len did not match len in L2 length/type) */
	CVMX_PIP_DAT_ERR =
		11ull, /* RGM               11 = Frame error (some or all data bits marked err) */
	CVMX_PIP_DIP_ERR = 11ull, /*     SPI           11 = DIP4 error */
	CVMX_PIP_SKIP_ERR =
		12ull, /* RGM               12 = packet was not large enough to pass the skipper - no inspection could occur */
	CVMX_PIP_NIBBLE_ERR =
		13ull, /* RGM               13 = studder error (data not repeated - 100M and 10M only) */
	CVMX_PIP_PIP_FCS = 16L, /* RGM+SPI           16 = FCS error */
	CVMX_PIP_PIP_SKIP_ERR =
		17L, /* RGM+SPI+PCI       17 = packet was not large enough to pass the skipper - no inspection could occur */
	CVMX_PIP_PIP_L2_MAL_HDR =
		18L, /* RGM+SPI+PCI       18 = malformed l2 (packet not long enough to cover L2 hdr) */
	CVMX_PIP_PUNY_ERR =
		47L /* SGMII             47 = PUNY error (packet was 4B or less when FCS stripping is enabled) */
	/* NOTES
	 *       xx = late collision (data received before collision)
	 *            late collisions cannot be detected by the receiver
	 *            they would appear as JAM bits which would appear as bad FCS
	 *            or carrier extend error which is CVMX_PIP_EXTEND_ERR
	 */
} cvmx_pip_rcv_err_t;

/**
 * This defines the err_code field errors in the work Q entry
 */
typedef union {
	cvmx_pip_l4_err_t l4_err;
	cvmx_pip_ip_exc_t ip_exc;
	cvmx_pip_rcv_err_t rcv_err;
} cvmx_pip_err_t;

/**
 * Status statistics for a port
 */
typedef struct {
	u64 dropped_octets;
	u64 dropped_packets;
	u64 pci_raw_packets;
	u64 octets;
	u64 packets;
	u64 multicast_packets;
	u64 broadcast_packets;
	u64 len_64_packets;
	u64 len_65_127_packets;
	u64 len_128_255_packets;
	u64 len_256_511_packets;
	u64 len_512_1023_packets;
	u64 len_1024_1518_packets;
	u64 len_1519_max_packets;
	u64 fcs_align_err_packets;
	u64 runt_packets;
	u64 runt_crc_packets;
	u64 oversize_packets;
	u64 oversize_crc_packets;
	u64 inb_packets;
	u64 inb_octets;
	u64 inb_errors;
	u64 mcast_l2_red_packets;
	u64 bcast_l2_red_packets;
	u64 mcast_l3_red_packets;
	u64 bcast_l3_red_packets;
} cvmx_pip_port_status_t;

/**
 * Definition of the PIP custom header that can be prepended
 * to a packet by external hardware.
 */
typedef union {
	u64 u64;
	struct {
		u64 rawfull : 1;
		u64 reserved0 : 5;
		cvmx_pip_port_parse_mode_t parse_mode : 2;
		u64 reserved1 : 1;
		u64 skip_len : 7;
		u64 grpext : 2;
		u64 nqos : 1;
		u64 ngrp : 1;
		u64 ntt : 1;
		u64 ntag : 1;
		u64 qos : 3;
		u64 grp : 4;
		u64 rs : 1;
		cvmx_pow_tag_type_t tag_type : 2;
		u64 tag : 32;
	} s;
} cvmx_pip_pkt_inst_hdr_t;

enum cvmx_pki_pcam_match {
	CVMX_PKI_PCAM_MATCH_IP,
	CVMX_PKI_PCAM_MATCH_IPV4,
	CVMX_PKI_PCAM_MATCH_IPV6,
	CVMX_PKI_PCAM_MATCH_TCP
};

/* CSR typedefs have been moved to cvmx-pip-defs.h */
static inline int cvmx_pip_config_watcher(int index, int type, u16 match, u16 mask, int grp,
					  int qos)
{
	if (index >= CVMX_PIP_NUM_WATCHERS) {
		debug("ERROR: pip watcher %d is > than supported\n", index);
		return -1;
	}
	if (octeon_has_feature(OCTEON_FEATURE_PKI)) {
		/* store in software for now, only when the watcher is enabled program the entry*/
		if (type == CVMX_PIP_QOS_WATCH_PROTNH) {
			qos_watcher[index].field = CVMX_PKI_PCAM_TERM_L3_FLAGS;
			qos_watcher[index].data = (u32)(match << 16);
			qos_watcher[index].data_mask = (u32)(mask << 16);
			qos_watcher[index].advance = 0;
		} else if (type == CVMX_PIP_QOS_WATCH_TCP) {
			qos_watcher[index].field = CVMX_PKI_PCAM_TERM_L4_PORT;
			qos_watcher[index].data = 0x060000;
			qos_watcher[index].data |= (u32)match;
			qos_watcher[index].data_mask = (u32)(mask);
			qos_watcher[index].advance = 0;
		} else if (type == CVMX_PIP_QOS_WATCH_UDP) {
			qos_watcher[index].field = CVMX_PKI_PCAM_TERM_L4_PORT;
			qos_watcher[index].data = 0x110000;
			qos_watcher[index].data |= (u32)match;
			qos_watcher[index].data_mask = (u32)(mask);
			qos_watcher[index].advance = 0;
		} else if (type == 0x4 /*CVMX_PIP_QOS_WATCH_ETHERTYPE*/) {
			qos_watcher[index].field = CVMX_PKI_PCAM_TERM_ETHTYPE0;
			if (match == 0x8100) {
				debug("ERROR: default vlan entry already exist, cant set watcher\n");
				return -1;
			}
			qos_watcher[index].data = (u32)(match << 16);
			qos_watcher[index].data_mask = (u32)(mask << 16);
			qos_watcher[index].advance = 4;
		} else {
			debug("ERROR: Unsupported watcher type %d\n", type);
			return -1;
		}
		if (grp >= 32) {
			debug("ERROR: grp %d out of range for backward compat 78xx\n", grp);
			return -1;
		}
		qos_watcher[index].sso_grp = (u8)(grp << 3 | qos);
		qos_watcher[index].configured = 1;
	} else {
		/* Implement it later */
	}
	return 0;
}

static inline int __cvmx_pip_set_tag_type(int node, int style, int tag_type, int field)
{
	struct cvmx_pki_style_config style_cfg;
	int style_num;
	int pcam_offset;
	int bank;
	struct cvmx_pki_pcam_input pcam_input;
	struct cvmx_pki_pcam_action pcam_action;

	/* All other style parameters remain same except tag type */
	cvmx_pki_read_style_config(node, style, CVMX_PKI_CLUSTER_ALL, &style_cfg);
	style_cfg.parm_cfg.tag_type = (enum cvmx_sso_tag_type)tag_type;
	style_num = cvmx_pki_style_alloc(node, -1);
	if (style_num < 0) {
		debug("ERROR: style not available to set tag type\n");
		return -1;
	}
	cvmx_pki_write_style_config(node, style_num, CVMX_PKI_CLUSTER_ALL, &style_cfg);
	memset(&pcam_input, 0, sizeof(pcam_input));
	memset(&pcam_action, 0, sizeof(pcam_action));
	pcam_input.style = style;
	pcam_input.style_mask = 0xff;
	if (field == CVMX_PKI_PCAM_MATCH_IP) {
		pcam_input.field = CVMX_PKI_PCAM_TERM_ETHTYPE0;
		pcam_input.field_mask = 0xff;
		pcam_input.data = 0x08000000;
		pcam_input.data_mask = 0xffff0000;
		pcam_action.pointer_advance = 4;
		/* legacy will write to all clusters*/
		bank = 0;
		pcam_offset = cvmx_pki_pcam_entry_alloc(node, CVMX_PKI_FIND_AVAL_ENTRY, bank,
							CVMX_PKI_CLUSTER_ALL);
		if (pcam_offset < 0) {
			debug("ERROR: pcam entry not available to enable qos watcher\n");
			cvmx_pki_style_free(node, style_num);
			return -1;
		}
		pcam_action.parse_mode_chg = CVMX_PKI_PARSE_NO_CHG;
		pcam_action.layer_type_set = CVMX_PKI_LTYPE_E_NONE;
		pcam_action.style_add = (u8)(style_num - style);
		cvmx_pki_pcam_write_entry(node, pcam_offset, CVMX_PKI_CLUSTER_ALL, pcam_input,
					  pcam_action);
		field = CVMX_PKI_PCAM_MATCH_IPV6;
	}
	if (field == CVMX_PKI_PCAM_MATCH_IPV4) {
		pcam_input.field = CVMX_PKI_PCAM_TERM_ETHTYPE0;
		pcam_input.field_mask = 0xff;
		pcam_input.data = 0x08000000;
		pcam_input.data_mask = 0xffff0000;
		pcam_action.pointer_advance = 4;
	} else if (field == CVMX_PKI_PCAM_MATCH_IPV6) {
		pcam_input.field = CVMX_PKI_PCAM_TERM_ETHTYPE0;
		pcam_input.field_mask = 0xff;
		pcam_input.data = 0x86dd00000;
		pcam_input.data_mask = 0xffff0000;
		pcam_action.pointer_advance = 4;
	} else if (field == CVMX_PKI_PCAM_MATCH_TCP) {
		pcam_input.field = CVMX_PKI_PCAM_TERM_L4_PORT;
		pcam_input.field_mask = 0xff;
		pcam_input.data = 0x60000;
		pcam_input.data_mask = 0xff0000;
		pcam_action.pointer_advance = 0;
	}
	pcam_action.parse_mode_chg = CVMX_PKI_PARSE_NO_CHG;
	pcam_action.layer_type_set = CVMX_PKI_LTYPE_E_NONE;
	pcam_action.style_add = (u8)(style_num - style);
	bank = pcam_input.field & 0x01;
	pcam_offset = cvmx_pki_pcam_entry_alloc(node, CVMX_PKI_FIND_AVAL_ENTRY, bank,
						CVMX_PKI_CLUSTER_ALL);
	if (pcam_offset < 0) {
		debug("ERROR: pcam entry not available to enable qos watcher\n");
		cvmx_pki_style_free(node, style_num);
		return -1;
	}
	cvmx_pki_pcam_write_entry(node, pcam_offset, CVMX_PKI_CLUSTER_ALL, pcam_input, pcam_action);
	return style_num;
}

/* Only for legacy internal use */
static inline int __cvmx_pip_enable_watcher_78xx(int node, int index, int style)
{
	struct cvmx_pki_style_config style_cfg;
	struct cvmx_pki_qpg_config qpg_cfg;
	struct cvmx_pki_pcam_input pcam_input;
	struct cvmx_pki_pcam_action pcam_action;
	int style_num;
	int qpg_offset;
	int pcam_offset;
	int bank;

	if (!qos_watcher[index].configured) {
		debug("ERROR: qos watcher %d should be configured before enable\n", index);
		return -1;
	}
	/* All other style parameters remain same except grp and qos and qps base */
	cvmx_pki_read_style_config(node, style, CVMX_PKI_CLUSTER_ALL, &style_cfg);
	cvmx_pki_read_qpg_entry(node, style_cfg.parm_cfg.qpg_base, &qpg_cfg);
	qpg_cfg.qpg_base = CVMX_PKI_FIND_AVAL_ENTRY;
	qpg_cfg.grp_ok = qos_watcher[index].sso_grp;
	qpg_cfg.grp_bad = qos_watcher[index].sso_grp;
	qpg_offset = cvmx_helper_pki_set_qpg_entry(node, &qpg_cfg);
	if (qpg_offset == -1) {
		debug("Warning: no new qpg entry available to enable watcher\n");
		return -1;
	}
	/* try to reserve the style, if it is not configured already, reserve
	   and configure it */
	style_cfg.parm_cfg.qpg_base = qpg_offset;
	style_num = cvmx_pki_style_alloc(node, -1);
	if (style_num < 0) {
		debug("ERROR: style not available to enable qos watcher\n");
		cvmx_pki_qpg_entry_free(node, qpg_offset, 1);
		return -1;
	}
	cvmx_pki_write_style_config(node, style_num, CVMX_PKI_CLUSTER_ALL, &style_cfg);
	/* legacy will write to all clusters*/
	bank = qos_watcher[index].field & 0x01;
	pcam_offset = cvmx_pki_pcam_entry_alloc(node, CVMX_PKI_FIND_AVAL_ENTRY, bank,
						CVMX_PKI_CLUSTER_ALL);
	if (pcam_offset < 0) {
		debug("ERROR: pcam entry not available to enable qos watcher\n");
		cvmx_pki_style_free(node, style_num);
		cvmx_pki_qpg_entry_free(node, qpg_offset, 1);
		return -1;
	}
	memset(&pcam_input, 0, sizeof(pcam_input));
	memset(&pcam_action, 0, sizeof(pcam_action));
	pcam_input.style = style;
	pcam_input.style_mask = 0xff;
	pcam_input.field = qos_watcher[index].field;
	pcam_input.field_mask = 0xff;
	pcam_input.data = qos_watcher[index].data;
	pcam_input.data_mask = qos_watcher[index].data_mask;
	pcam_action.parse_mode_chg = CVMX_PKI_PARSE_NO_CHG;
	pcam_action.layer_type_set = CVMX_PKI_LTYPE_E_NONE;
	pcam_action.style_add = (u8)(style_num - style);
	pcam_action.pointer_advance = qos_watcher[index].advance;
	cvmx_pki_pcam_write_entry(node, pcam_offset, CVMX_PKI_CLUSTER_ALL, pcam_input, pcam_action);
	return 0;
}

/**
 * Configure an ethernet input port
 *
 * @param ipd_port Port number to configure
 * @param port_cfg Port hardware configuration
 * @param port_tag_cfg Port POW tagging configuration
 */
static inline void cvmx_pip_config_port(u64 ipd_port, cvmx_pip_prt_cfgx_t port_cfg,
					cvmx_pip_prt_tagx_t port_tag_cfg)
{
	struct cvmx_pki_qpg_config qpg_cfg;
	int qpg_offset;
	u8 tcp_tag = 0xff;
	u8 ip_tag = 0xaa;
	int style, nstyle, n4style, n6style;

	if (octeon_has_feature(OCTEON_FEATURE_PKI)) {
		struct cvmx_pki_port_config pki_prt_cfg;
		struct cvmx_xport xp = cvmx_helper_ipd_port_to_xport(ipd_port);

		cvmx_pki_get_port_config(ipd_port, &pki_prt_cfg);
		style = pki_prt_cfg.pkind_cfg.initial_style;
		if (port_cfg.s.ih_pri || port_cfg.s.vlan_len || port_cfg.s.pad_len)
			debug("Warning: 78xx: use different config for this option\n");
		pki_prt_cfg.style_cfg.parm_cfg.minmax_sel = port_cfg.s.len_chk_sel;
		pki_prt_cfg.style_cfg.parm_cfg.lenerr_en = port_cfg.s.lenerr_en;
		pki_prt_cfg.style_cfg.parm_cfg.maxerr_en = port_cfg.s.maxerr_en;
		pki_prt_cfg.style_cfg.parm_cfg.minerr_en = port_cfg.s.minerr_en;
		pki_prt_cfg.style_cfg.parm_cfg.fcs_chk = port_cfg.s.crc_en;
		if (port_cfg.s.grp_wat || port_cfg.s.qos_wat || port_cfg.s.grp_wat_47 ||
		    port_cfg.s.qos_wat_47) {
			u8 group_mask = (u8)(port_cfg.s.grp_wat | (u8)(port_cfg.s.grp_wat_47 << 4));
			u8 qos_mask = (u8)(port_cfg.s.qos_wat | (u8)(port_cfg.s.qos_wat_47 << 4));
			int i;

			for (i = 0; i < CVMX_PIP_NUM_WATCHERS; i++) {
				if ((group_mask & (1 << i)) || (qos_mask & (1 << i)))
					__cvmx_pip_enable_watcher_78xx(xp.node, i, style);
			}
		}
		if (port_tag_cfg.s.tag_mode) {
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				cvmx_printf("Warning: mask tag is not supported in 78xx pass1\n");
			else {
			}
			/* need to implement for 78xx*/
		}
		if (port_cfg.s.tag_inc)
			debug("Warning: 78xx uses differnet method for tag generation\n");
		pki_prt_cfg.style_cfg.parm_cfg.rawdrp = port_cfg.s.rawdrp;
		pki_prt_cfg.pkind_cfg.parse_en.inst_hdr = port_cfg.s.inst_hdr;
		if (port_cfg.s.hg_qos)
			pki_prt_cfg.style_cfg.parm_cfg.qpg_qos = CVMX_PKI_QPG_QOS_HIGIG;
		else if (port_cfg.s.qos_vlan)
			pki_prt_cfg.style_cfg.parm_cfg.qpg_qos = CVMX_PKI_QPG_QOS_VLAN;
		else if (port_cfg.s.qos_diff)
			pki_prt_cfg.style_cfg.parm_cfg.qpg_qos = CVMX_PKI_QPG_QOS_DIFFSERV;
		if (port_cfg.s.qos_vod)
			debug("Warning: 78xx needs pcam entries installed to achieve qos_vod\n");
		if (port_cfg.s.qos) {
			cvmx_pki_read_qpg_entry(xp.node, pki_prt_cfg.style_cfg.parm_cfg.qpg_base,
						&qpg_cfg);
			qpg_cfg.qpg_base = CVMX_PKI_FIND_AVAL_ENTRY;
			qpg_cfg.grp_ok |= port_cfg.s.qos;
			qpg_cfg.grp_bad |= port_cfg.s.qos;
			qpg_offset = cvmx_helper_pki_set_qpg_entry(xp.node, &qpg_cfg);
			if (qpg_offset == -1)
				debug("Warning: no new qpg entry available, will not modify qos\n");
			else
				pki_prt_cfg.style_cfg.parm_cfg.qpg_base = qpg_offset;
		}
		if (port_tag_cfg.s.grp != pki_dflt_sso_grp[xp.node].group) {
			cvmx_pki_read_qpg_entry(xp.node, pki_prt_cfg.style_cfg.parm_cfg.qpg_base,
						&qpg_cfg);
			qpg_cfg.qpg_base = CVMX_PKI_FIND_AVAL_ENTRY;
			qpg_cfg.grp_ok |= (u8)(port_tag_cfg.s.grp << 3);
			qpg_cfg.grp_bad |= (u8)(port_tag_cfg.s.grp << 3);
			qpg_offset = cvmx_helper_pki_set_qpg_entry(xp.node, &qpg_cfg);
			if (qpg_offset == -1)
				debug("Warning: no new qpg entry available, will not modify group\n");
			else
				pki_prt_cfg.style_cfg.parm_cfg.qpg_base = qpg_offset;
		}
		pki_prt_cfg.pkind_cfg.parse_en.dsa_en = port_cfg.s.dsa_en;
		pki_prt_cfg.pkind_cfg.parse_en.hg_en = port_cfg.s.higig_en;
		pki_prt_cfg.style_cfg.tag_cfg.tag_fields.layer_c_src =
			port_tag_cfg.s.ip6_src_flag | port_tag_cfg.s.ip4_src_flag;
		pki_prt_cfg.style_cfg.tag_cfg.tag_fields.layer_c_dst =
			port_tag_cfg.s.ip6_dst_flag | port_tag_cfg.s.ip4_dst_flag;
		pki_prt_cfg.style_cfg.tag_cfg.tag_fields.ip_prot_nexthdr =
			port_tag_cfg.s.ip6_nxth_flag | port_tag_cfg.s.ip4_pctl_flag;
		pki_prt_cfg.style_cfg.tag_cfg.tag_fields.layer_d_src =
			port_tag_cfg.s.ip6_sprt_flag | port_tag_cfg.s.ip4_sprt_flag;
		pki_prt_cfg.style_cfg.tag_cfg.tag_fields.layer_d_dst =
			port_tag_cfg.s.ip6_dprt_flag | port_tag_cfg.s.ip4_dprt_flag;
		pki_prt_cfg.style_cfg.tag_cfg.tag_fields.input_port = port_tag_cfg.s.inc_prt_flag;
		pki_prt_cfg.style_cfg.tag_cfg.tag_fields.first_vlan = port_tag_cfg.s.inc_vlan;
		pki_prt_cfg.style_cfg.tag_cfg.tag_fields.second_vlan = port_tag_cfg.s.inc_vs;

		if (port_tag_cfg.s.tcp6_tag_type == port_tag_cfg.s.tcp4_tag_type)
			tcp_tag = port_tag_cfg.s.tcp6_tag_type;
		if (port_tag_cfg.s.ip6_tag_type == port_tag_cfg.s.ip4_tag_type)
			ip_tag = port_tag_cfg.s.ip6_tag_type;
		pki_prt_cfg.style_cfg.parm_cfg.tag_type =
			(enum cvmx_sso_tag_type)port_tag_cfg.s.non_tag_type;
		if (tcp_tag == ip_tag && tcp_tag == port_tag_cfg.s.non_tag_type)
			pki_prt_cfg.style_cfg.parm_cfg.tag_type = (enum cvmx_sso_tag_type)tcp_tag;
		else if (tcp_tag == ip_tag) {
			/* allocate and copy style */
			/* modify tag type */
			/*pcam entry for ip6 && ip4 match*/
			/* default is non tag type */
			__cvmx_pip_set_tag_type(xp.node, style, ip_tag, CVMX_PKI_PCAM_MATCH_IP);
		} else if (ip_tag == port_tag_cfg.s.non_tag_type) {
			/* allocate and copy style */
			/* modify tag type */
			/*pcam entry for tcp6 & tcp4 match*/
			/* default is non tag type */
			__cvmx_pip_set_tag_type(xp.node, style, tcp_tag, CVMX_PKI_PCAM_MATCH_TCP);
		} else {
			if (ip_tag != 0xaa) {
				nstyle = __cvmx_pip_set_tag_type(xp.node, style, ip_tag,
								 CVMX_PKI_PCAM_MATCH_IP);
				if (tcp_tag != 0xff)
					__cvmx_pip_set_tag_type(xp.node, nstyle, tcp_tag,
								CVMX_PKI_PCAM_MATCH_TCP);
				else {
					n4style = __cvmx_pip_set_tag_type(xp.node, nstyle, ip_tag,
									  CVMX_PKI_PCAM_MATCH_IPV4);
					__cvmx_pip_set_tag_type(xp.node, n4style,
								port_tag_cfg.s.tcp4_tag_type,
								CVMX_PKI_PCAM_MATCH_TCP);
					n6style = __cvmx_pip_set_tag_type(xp.node, nstyle, ip_tag,
									  CVMX_PKI_PCAM_MATCH_IPV6);
					__cvmx_pip_set_tag_type(xp.node, n6style,
								port_tag_cfg.s.tcp6_tag_type,
								CVMX_PKI_PCAM_MATCH_TCP);
				}
			} else {
				n4style = __cvmx_pip_set_tag_type(xp.node, style,
								  port_tag_cfg.s.ip4_tag_type,
								  CVMX_PKI_PCAM_MATCH_IPV4);
				n6style = __cvmx_pip_set_tag_type(xp.node, style,
								  port_tag_cfg.s.ip6_tag_type,
								  CVMX_PKI_PCAM_MATCH_IPV6);
				if (tcp_tag != 0xff) {
					__cvmx_pip_set_tag_type(xp.node, n4style, tcp_tag,
								CVMX_PKI_PCAM_MATCH_TCP);
					__cvmx_pip_set_tag_type(xp.node, n6style, tcp_tag,
								CVMX_PKI_PCAM_MATCH_TCP);
				} else {
					__cvmx_pip_set_tag_type(xp.node, n4style,
								port_tag_cfg.s.tcp4_tag_type,
								CVMX_PKI_PCAM_MATCH_TCP);
					__cvmx_pip_set_tag_type(xp.node, n6style,
								port_tag_cfg.s.tcp6_tag_type,
								CVMX_PKI_PCAM_MATCH_TCP);
				}
			}
		}
		pki_prt_cfg.style_cfg.parm_cfg.qpg_dis_padd = !port_tag_cfg.s.portadd_en;

		if (port_cfg.s.mode == 0x1)
			pki_prt_cfg.pkind_cfg.initial_parse_mode = CVMX_PKI_PARSE_LA_TO_LG;
		else if (port_cfg.s.mode == 0x2)
			pki_prt_cfg.pkind_cfg.initial_parse_mode = CVMX_PKI_PARSE_LC_TO_LG;
		else
			pki_prt_cfg.pkind_cfg.initial_parse_mode = CVMX_PKI_PARSE_NOTHING;
		/* This is only for backward compatibility, not all the parameters are supported in 78xx */
		cvmx_pki_set_port_config(ipd_port, &pki_prt_cfg);
	} else {
		if (octeon_has_feature(OCTEON_FEATURE_PKND)) {
			int interface, index, pknd;

			interface = cvmx_helper_get_interface_num(ipd_port);
			index = cvmx_helper_get_interface_index_num(ipd_port);
			pknd = cvmx_helper_get_pknd(interface, index);

			ipd_port = pknd; /* overload port_num with pknd */
		}
		csr_wr(CVMX_PIP_PRT_CFGX(ipd_port), port_cfg.u64);
		csr_wr(CVMX_PIP_PRT_TAGX(ipd_port), port_tag_cfg.u64);
	}
}

/**
 * Configure the VLAN priority to QoS queue mapping.
 *
 * @param vlan_priority
 *               VLAN priority (0-7)
 * @param qos    QoS queue for packets matching this watcher
 */
static inline void cvmx_pip_config_vlan_qos(u64 vlan_priority, u64 qos)
{
	if (!octeon_has_feature(OCTEON_FEATURE_PKND)) {
		cvmx_pip_qos_vlanx_t pip_qos_vlanx;

		pip_qos_vlanx.u64 = 0;
		pip_qos_vlanx.s.qos = qos;
		csr_wr(CVMX_PIP_QOS_VLANX(vlan_priority), pip_qos_vlanx.u64);
	}
}

/**
 * Configure the Diffserv to QoS queue mapping.
 *
 * @param diffserv Diffserv field value (0-63)
 * @param qos      QoS queue for packets matching this watcher
 */
static inline void cvmx_pip_config_diffserv_qos(u64 diffserv, u64 qos)
{
	if (!octeon_has_feature(OCTEON_FEATURE_PKND)) {
		cvmx_pip_qos_diffx_t pip_qos_diffx;

		pip_qos_diffx.u64 = 0;
		pip_qos_diffx.s.qos = qos;
		csr_wr(CVMX_PIP_QOS_DIFFX(diffserv), pip_qos_diffx.u64);
	}
}

/**
 * Get the status counters for a port for older non PKI chips.
 *
 * @param port_num Port number (ipd_port) to get statistics for.
 * @param clear    Set to 1 to clear the counters after they are read
 * @param status   Where to put the results.
 */
static inline void cvmx_pip_get_port_stats(u64 port_num, u64 clear, cvmx_pip_port_status_t *status)
{
	cvmx_pip_stat_ctl_t pip_stat_ctl;
	cvmx_pip_stat0_prtx_t stat0;
	cvmx_pip_stat1_prtx_t stat1;
	cvmx_pip_stat2_prtx_t stat2;
	cvmx_pip_stat3_prtx_t stat3;
	cvmx_pip_stat4_prtx_t stat4;
	cvmx_pip_stat5_prtx_t stat5;
	cvmx_pip_stat6_prtx_t stat6;
	cvmx_pip_stat7_prtx_t stat7;
	cvmx_pip_stat8_prtx_t stat8;
	cvmx_pip_stat9_prtx_t stat9;
	cvmx_pip_stat10_x_t stat10;
	cvmx_pip_stat11_x_t stat11;
	cvmx_pip_stat_inb_pktsx_t pip_stat_inb_pktsx;
	cvmx_pip_stat_inb_octsx_t pip_stat_inb_octsx;
	cvmx_pip_stat_inb_errsx_t pip_stat_inb_errsx;
	int interface = cvmx_helper_get_interface_num(port_num);
	int index = cvmx_helper_get_interface_index_num(port_num);

	pip_stat_ctl.u64 = 0;
	pip_stat_ctl.s.rdclr = clear;
	csr_wr(CVMX_PIP_STAT_CTL, pip_stat_ctl.u64);

	if (octeon_has_feature(OCTEON_FEATURE_PKND)) {
		int pknd = cvmx_helper_get_pknd(interface, index);
		/*
		 * PIP_STAT_CTL[MODE] 0 means pkind.
		 */
		stat0.u64 = csr_rd(CVMX_PIP_STAT0_X(pknd));
		stat1.u64 = csr_rd(CVMX_PIP_STAT1_X(pknd));
		stat2.u64 = csr_rd(CVMX_PIP_STAT2_X(pknd));
		stat3.u64 = csr_rd(CVMX_PIP_STAT3_X(pknd));
		stat4.u64 = csr_rd(CVMX_PIP_STAT4_X(pknd));
		stat5.u64 = csr_rd(CVMX_PIP_STAT5_X(pknd));
		stat6.u64 = csr_rd(CVMX_PIP_STAT6_X(pknd));
		stat7.u64 = csr_rd(CVMX_PIP_STAT7_X(pknd));
		stat8.u64 = csr_rd(CVMX_PIP_STAT8_X(pknd));
		stat9.u64 = csr_rd(CVMX_PIP_STAT9_X(pknd));
		stat10.u64 = csr_rd(CVMX_PIP_STAT10_X(pknd));
		stat11.u64 = csr_rd(CVMX_PIP_STAT11_X(pknd));
	} else {
		if (port_num >= 40) {
			stat0.u64 = csr_rd(CVMX_PIP_XSTAT0_PRTX(port_num));
			stat1.u64 = csr_rd(CVMX_PIP_XSTAT1_PRTX(port_num));
			stat2.u64 = csr_rd(CVMX_PIP_XSTAT2_PRTX(port_num));
			stat3.u64 = csr_rd(CVMX_PIP_XSTAT3_PRTX(port_num));
			stat4.u64 = csr_rd(CVMX_PIP_XSTAT4_PRTX(port_num));
			stat5.u64 = csr_rd(CVMX_PIP_XSTAT5_PRTX(port_num));
			stat6.u64 = csr_rd(CVMX_PIP_XSTAT6_PRTX(port_num));
			stat7.u64 = csr_rd(CVMX_PIP_XSTAT7_PRTX(port_num));
			stat8.u64 = csr_rd(CVMX_PIP_XSTAT8_PRTX(port_num));
			stat9.u64 = csr_rd(CVMX_PIP_XSTAT9_PRTX(port_num));
			if (OCTEON_IS_MODEL(OCTEON_CN6XXX)) {
				stat10.u64 = csr_rd(CVMX_PIP_XSTAT10_PRTX(port_num));
				stat11.u64 = csr_rd(CVMX_PIP_XSTAT11_PRTX(port_num));
			}
		} else {
			stat0.u64 = csr_rd(CVMX_PIP_STAT0_PRTX(port_num));
			stat1.u64 = csr_rd(CVMX_PIP_STAT1_PRTX(port_num));
			stat2.u64 = csr_rd(CVMX_PIP_STAT2_PRTX(port_num));
			stat3.u64 = csr_rd(CVMX_PIP_STAT3_PRTX(port_num));
			stat4.u64 = csr_rd(CVMX_PIP_STAT4_PRTX(port_num));
			stat5.u64 = csr_rd(CVMX_PIP_STAT5_PRTX(port_num));
			stat6.u64 = csr_rd(CVMX_PIP_STAT6_PRTX(port_num));
			stat7.u64 = csr_rd(CVMX_PIP_STAT7_PRTX(port_num));
			stat8.u64 = csr_rd(CVMX_PIP_STAT8_PRTX(port_num));
			stat9.u64 = csr_rd(CVMX_PIP_STAT9_PRTX(port_num));
			if (OCTEON_IS_OCTEON2() || OCTEON_IS_MODEL(OCTEON_CN70XX)) {
				stat10.u64 = csr_rd(CVMX_PIP_STAT10_PRTX(port_num));
				stat11.u64 = csr_rd(CVMX_PIP_STAT11_PRTX(port_num));
			}
		}
	}
	if (octeon_has_feature(OCTEON_FEATURE_PKND)) {
		int pknd = cvmx_helper_get_pknd(interface, index);

		pip_stat_inb_pktsx.u64 = csr_rd(CVMX_PIP_STAT_INB_PKTS_PKNDX(pknd));
		pip_stat_inb_octsx.u64 = csr_rd(CVMX_PIP_STAT_INB_OCTS_PKNDX(pknd));
		pip_stat_inb_errsx.u64 = csr_rd(CVMX_PIP_STAT_INB_ERRS_PKNDX(pknd));
	} else {
		pip_stat_inb_pktsx.u64 = csr_rd(CVMX_PIP_STAT_INB_PKTSX(port_num));
		pip_stat_inb_octsx.u64 = csr_rd(CVMX_PIP_STAT_INB_OCTSX(port_num));
		pip_stat_inb_errsx.u64 = csr_rd(CVMX_PIP_STAT_INB_ERRSX(port_num));
	}

	status->dropped_octets = stat0.s.drp_octs;
	status->dropped_packets = stat0.s.drp_pkts;
	status->octets = stat1.s.octs;
	status->pci_raw_packets = stat2.s.raw;
	status->packets = stat2.s.pkts;
	status->multicast_packets = stat3.s.mcst;
	status->broadcast_packets = stat3.s.bcst;
	status->len_64_packets = stat4.s.h64;
	status->len_65_127_packets = stat4.s.h65to127;
	status->len_128_255_packets = stat5.s.h128to255;
	status->len_256_511_packets = stat5.s.h256to511;
	status->len_512_1023_packets = stat6.s.h512to1023;
	status->len_1024_1518_packets = stat6.s.h1024to1518;
	status->len_1519_max_packets = stat7.s.h1519;
	status->fcs_align_err_packets = stat7.s.fcs;
	status->runt_packets = stat8.s.undersz;
	status->runt_crc_packets = stat8.s.frag;
	status->oversize_packets = stat9.s.oversz;
	status->oversize_crc_packets = stat9.s.jabber;
	if (OCTEON_IS_OCTEON2() || OCTEON_IS_MODEL(OCTEON_CN70XX)) {
		status->mcast_l2_red_packets = stat10.s.mcast;
		status->bcast_l2_red_packets = stat10.s.bcast;
		status->mcast_l3_red_packets = stat11.s.mcast;
		status->bcast_l3_red_packets = stat11.s.bcast;
	}
	status->inb_packets = pip_stat_inb_pktsx.s.pkts;
	status->inb_octets = pip_stat_inb_octsx.s.octs;
	status->inb_errors = pip_stat_inb_errsx.s.errs;
}

/**
 * Get the status counters for a port.
 *
 * @param port_num Port number (ipd_port) to get statistics for.
 * @param clear    Set to 1 to clear the counters after they are read
 * @param status   Where to put the results.
 */
static inline void cvmx_pip_get_port_status(u64 port_num, u64 clear, cvmx_pip_port_status_t *status)
{
	if (octeon_has_feature(OCTEON_FEATURE_PKI)) {
		unsigned int node = cvmx_get_node_num();

		cvmx_pki_get_port_stats(node, port_num, (struct cvmx_pki_port_stats *)status);
	} else {
		cvmx_pip_get_port_stats(port_num, clear, status);
	}
}

/**
 * Configure the hardware CRC engine
 *
 * @param interface Interface to configure (0 or 1)
 * @param invert_result
 *                 Invert the result of the CRC
 * @param reflect  Reflect
 * @param initialization_vector
 *                 CRC initialization vector
 */
static inline void cvmx_pip_config_crc(u64 interface, u64 invert_result, u64 reflect,
				       u32 initialization_vector)
{
	/* Only CN38XX & CN58XX */
}

/**
 * Clear all bits in a tag mask. This should be called on
 * startup before any calls to cvmx_pip_tag_mask_set. Each bit
 * set in the final mask represent a byte used in the packet for
 * tag generation.
 *
 * @param mask_index Which tag mask to clear (0..3)
 */
static inline void cvmx_pip_tag_mask_clear(u64 mask_index)
{
	u64 index;
	cvmx_pip_tag_incx_t pip_tag_incx;

	pip_tag_incx.u64 = 0;
	pip_tag_incx.s.en = 0;
	for (index = mask_index * 16; index < (mask_index + 1) * 16; index++)
		csr_wr(CVMX_PIP_TAG_INCX(index), pip_tag_incx.u64);
}

/**
 * Sets a range of bits in the tag mask. The tag mask is used
 * when the cvmx_pip_port_tag_cfg_t tag_mode is non zero.
 * There are four separate masks that can be configured.
 *
 * @param mask_index Which tag mask to modify (0..3)
 * @param offset     Offset into the bitmask to set bits at. Use the GCC macro
 *                   offsetof() to determine the offsets into packet headers.
 *                   For example, offsetof(ethhdr, protocol) returns the offset
 *                   of the ethernet protocol field.  The bitmask selects which bytes
 *                   to include the the tag, with bit offset X selecting byte at offset X
 *                   from the beginning of the packet data.
 * @param len        Number of bytes to include. Usually this is the sizeof()
 *                   the field.
 */
static inline void cvmx_pip_tag_mask_set(u64 mask_index, u64 offset, u64 len)
{
	while (len--) {
		cvmx_pip_tag_incx_t pip_tag_incx;
		u64 index = mask_index * 16 + offset / 8;

		pip_tag_incx.u64 = csr_rd(CVMX_PIP_TAG_INCX(index));
		pip_tag_incx.s.en |= 0x80 >> (offset & 0x7);
		csr_wr(CVMX_PIP_TAG_INCX(index), pip_tag_incx.u64);
		offset++;
	}
}

/**
 * Set byte count for Max-Sized and Min Sized frame check.
 *
 * @param interface   Which interface to set the limit
 * @param max_size    Byte count for Max-Size frame check
 */
static inline void cvmx_pip_set_frame_check(int interface, u32 max_size)
{
	cvmx_pip_frm_len_chkx_t frm_len;

	/* max_size and min_size are passed as 0, reset to default values. */
	if (max_size < 1536)
		max_size = 1536;

	/* On CN68XX frame check is enabled for a pkind n and
	   PIP_PRT_CFG[len_chk_sel] selects which set of
	   MAXLEN/MINLEN to use. */
	if (octeon_has_feature(OCTEON_FEATURE_PKND)) {
		int port;
		int num_ports = cvmx_helper_ports_on_interface(interface);

		for (port = 0; port < num_ports; port++) {
			if (octeon_has_feature(OCTEON_FEATURE_PKI)) {
				int ipd_port;

				ipd_port = cvmx_helper_get_ipd_port(interface, port);
				cvmx_pki_set_max_frm_len(ipd_port, max_size);
			} else {
				int pknd;
				int sel;
				cvmx_pip_prt_cfgx_t config;

				pknd = cvmx_helper_get_pknd(interface, port);
				config.u64 = csr_rd(CVMX_PIP_PRT_CFGX(pknd));
				sel = config.s.len_chk_sel;
				frm_len.u64 = csr_rd(CVMX_PIP_FRM_LEN_CHKX(sel));
				frm_len.s.maxlen = max_size;
				csr_wr(CVMX_PIP_FRM_LEN_CHKX(sel), frm_len.u64);
			}
		}
	}
	/* on cn6xxx and cn7xxx models, PIP_FRM_LEN_CHK0 applies to
	 *     all incoming traffic */
	else if (OCTEON_IS_OCTEON2() || OCTEON_IS_MODEL(OCTEON_CN70XX)) {
		frm_len.u64 = csr_rd(CVMX_PIP_FRM_LEN_CHKX(0));
		frm_len.s.maxlen = max_size;
		csr_wr(CVMX_PIP_FRM_LEN_CHKX(0), frm_len.u64);
	}
}

/**
 * Initialize Bit Select Extractor config. Their are 8 bit positions and valids
 * to be used when using the corresponding extractor.
 *
 * @param bit     Bit Select Extractor to use
 * @param pos     Which position to update
 * @param val     The value to update the position with
 */
static inline void cvmx_pip_set_bsel_pos(int bit, int pos, int val)
{
	cvmx_pip_bsel_ext_posx_t bsel_pos;

	/* The bit select extractor is available in CN61XX and CN68XX pass2.0 onwards. */
	if (!octeon_has_feature(OCTEON_FEATURE_BIT_EXTRACTOR))
		return;

	if (bit < 0 || bit > 3) {
		debug("ERROR: cvmx_pip_set_bsel_pos: Invalid Bit-Select Extractor (%d) passed\n",
		      bit);
		return;
	}

	bsel_pos.u64 = csr_rd(CVMX_PIP_BSEL_EXT_POSX(bit));
	switch (pos) {
	case 0:
		bsel_pos.s.pos0_val = 1;
		bsel_pos.s.pos0 = val & 0x7f;
		break;
	case 1:
		bsel_pos.s.pos1_val = 1;
		bsel_pos.s.pos1 = val & 0x7f;
		break;
	case 2:
		bsel_pos.s.pos2_val = 1;
		bsel_pos.s.pos2 = val & 0x7f;
		break;
	case 3:
		bsel_pos.s.pos3_val = 1;
		bsel_pos.s.pos3 = val & 0x7f;
		break;
	case 4:
		bsel_pos.s.pos4_val = 1;
		bsel_pos.s.pos4 = val & 0x7f;
		break;
	case 5:
		bsel_pos.s.pos5_val = 1;
		bsel_pos.s.pos5 = val & 0x7f;
		break;
	case 6:
		bsel_pos.s.pos6_val = 1;
		bsel_pos.s.pos6 = val & 0x7f;
		break;
	case 7:
		bsel_pos.s.pos7_val = 1;
		bsel_pos.s.pos7 = val & 0x7f;
		break;
	default:
		debug("Warning: cvmx_pip_set_bsel_pos: Invalid pos(%d)\n", pos);
		break;
	}
	csr_wr(CVMX_PIP_BSEL_EXT_POSX(bit), bsel_pos.u64);
}

/**
 * Initialize offset and skip values to use by bit select extractor.

 * @param bit	Bit Select Extractor to use
 * @param offset	Offset to add to extractor mem addr to get final address
 *			to lookup table.
 * @param skip		Number of bytes to skip from start of packet 0-64
 */
static inline void cvmx_pip_bsel_config(int bit, int offset, int skip)
{
	cvmx_pip_bsel_ext_cfgx_t bsel_cfg;

	/* The bit select extractor is available in CN61XX and CN68XX pass2.0 onwards. */
	if (!octeon_has_feature(OCTEON_FEATURE_BIT_EXTRACTOR))
		return;

	bsel_cfg.u64 = csr_rd(CVMX_PIP_BSEL_EXT_CFGX(bit));
	bsel_cfg.s.offset = offset;
	bsel_cfg.s.skip = skip;
	csr_wr(CVMX_PIP_BSEL_EXT_CFGX(bit), bsel_cfg.u64);
}

/**
 * Get the entry for the Bit Select Extractor Table.
 * @param work   pointer to work queue entry
 * Return:       Index of the Bit Select Extractor Table
 */
static inline int cvmx_pip_get_bsel_table_index(cvmx_wqe_t *work)
{
	int bit = cvmx_wqe_get_port(work) & 0x3;
	/* Get the Bit select table index. */
	int index;
	int y;
	cvmx_pip_bsel_ext_cfgx_t bsel_cfg;
	cvmx_pip_bsel_ext_posx_t bsel_pos;

	/* The bit select extractor is available in CN61XX and CN68XX pass2.0 onwards. */
	if (!octeon_has_feature(OCTEON_FEATURE_BIT_EXTRACTOR))
		return -1;

	bsel_cfg.u64 = csr_rd(CVMX_PIP_BSEL_EXT_CFGX(bit));
	bsel_pos.u64 = csr_rd(CVMX_PIP_BSEL_EXT_POSX(bit));

	for (y = 0; y < 8; y++) {
		char *ptr = (char *)cvmx_phys_to_ptr(work->packet_ptr.s.addr);
		int bit_loc = 0;
		int bit;

		ptr += bsel_cfg.s.skip;
		switch (y) {
		case 0:
			ptr += (bsel_pos.s.pos0 >> 3);
			bit_loc = 7 - (bsel_pos.s.pos0 & 0x3);
			break;
		case 1:
			ptr += (bsel_pos.s.pos1 >> 3);
			bit_loc = 7 - (bsel_pos.s.pos1 & 0x3);
			break;
		case 2:
			ptr += (bsel_pos.s.pos2 >> 3);
			bit_loc = 7 - (bsel_pos.s.pos2 & 0x3);
			break;
		case 3:
			ptr += (bsel_pos.s.pos3 >> 3);
			bit_loc = 7 - (bsel_pos.s.pos3 & 0x3);
			break;
		case 4:
			ptr += (bsel_pos.s.pos4 >> 3);
			bit_loc = 7 - (bsel_pos.s.pos4 & 0x3);
			break;
		case 5:
			ptr += (bsel_pos.s.pos5 >> 3);
			bit_loc = 7 - (bsel_pos.s.pos5 & 0x3);
			break;
		case 6:
			ptr += (bsel_pos.s.pos6 >> 3);
			bit_loc = 7 - (bsel_pos.s.pos6 & 0x3);
			break;
		case 7:
			ptr += (bsel_pos.s.pos7 >> 3);
			bit_loc = 7 - (bsel_pos.s.pos7 & 0x3);
			break;
		}
		bit = (*ptr >> bit_loc) & 1;
		index |= bit << y;
	}
	index += bsel_cfg.s.offset;
	index &= 0x1ff;
	return index;
}

static inline int cvmx_pip_get_bsel_qos(cvmx_wqe_t *work)
{
	int index = cvmx_pip_get_bsel_table_index(work);
	cvmx_pip_bsel_tbl_entx_t bsel_tbl;

	/* The bit select extractor is available in CN61XX and CN68XX pass2.0 onwards. */
	if (!octeon_has_feature(OCTEON_FEATURE_BIT_EXTRACTOR))
		return -1;

	bsel_tbl.u64 = csr_rd(CVMX_PIP_BSEL_TBL_ENTX(index));

	return bsel_tbl.s.qos;
}

static inline int cvmx_pip_get_bsel_grp(cvmx_wqe_t *work)
{
	int index = cvmx_pip_get_bsel_table_index(work);
	cvmx_pip_bsel_tbl_entx_t bsel_tbl;

	/* The bit select extractor is available in CN61XX and CN68XX pass2.0 onwards. */
	if (!octeon_has_feature(OCTEON_FEATURE_BIT_EXTRACTOR))
		return -1;

	bsel_tbl.u64 = csr_rd(CVMX_PIP_BSEL_TBL_ENTX(index));

	return bsel_tbl.s.grp;
}

static inline int cvmx_pip_get_bsel_tt(cvmx_wqe_t *work)
{
	int index = cvmx_pip_get_bsel_table_index(work);
	cvmx_pip_bsel_tbl_entx_t bsel_tbl;

	/* The bit select extractor is available in CN61XX and CN68XX pass2.0 onwards. */
	if (!octeon_has_feature(OCTEON_FEATURE_BIT_EXTRACTOR))
		return -1;

	bsel_tbl.u64 = csr_rd(CVMX_PIP_BSEL_TBL_ENTX(index));

	return bsel_tbl.s.tt;
}

static inline int cvmx_pip_get_bsel_tag(cvmx_wqe_t *work)
{
	int index = cvmx_pip_get_bsel_table_index(work);
	int port = cvmx_wqe_get_port(work);
	int bit = port & 0x3;
	int upper_tag = 0;
	cvmx_pip_bsel_tbl_entx_t bsel_tbl;
	cvmx_pip_bsel_ext_cfgx_t bsel_cfg;
	cvmx_pip_prt_tagx_t prt_tag;

	/* The bit select extractor is available in CN61XX and CN68XX pass2.0 onwards. */
	if (!octeon_has_feature(OCTEON_FEATURE_BIT_EXTRACTOR))
		return -1;

	bsel_tbl.u64 = csr_rd(CVMX_PIP_BSEL_TBL_ENTX(index));
	bsel_cfg.u64 = csr_rd(CVMX_PIP_BSEL_EXT_CFGX(bit));

	prt_tag.u64 = csr_rd(CVMX_PIP_PRT_TAGX(port));
	if (prt_tag.s.inc_prt_flag == 0)
		upper_tag = bsel_cfg.s.upper_tag;
	return bsel_tbl.s.tag | ((bsel_cfg.s.tag << 8) & 0xff00) | ((upper_tag << 16) & 0xffff0000);
}

#endif /*  __CVMX_PIP_H__ */
