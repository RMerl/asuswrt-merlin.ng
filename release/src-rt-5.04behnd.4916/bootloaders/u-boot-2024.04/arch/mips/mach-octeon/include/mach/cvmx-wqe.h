/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * This header file defines the work queue entry (wqe) data structure.
 * Since this is a commonly used structure that depends on structures
 * from several hardware blocks, those definitions have been placed
 * in this file to create a single point of definition of the wqe
 * format.
 * Data structures are still named according to the block that they
 * relate to.
 */

#ifndef __CVMX_WQE_H__
#define __CVMX_WQE_H__

#include "cvmx-packet.h"
#include "cvmx-csr-enums.h"
#include "cvmx-pki-defs.h"
#include "cvmx-pip-defs.h"
#include "octeon-feature.h"

#define OCT_TAG_TYPE_STRING(x)						\
	(((x) == CVMX_POW_TAG_TYPE_ORDERED) ?				\
	 "ORDERED" :							\
	 (((x) == CVMX_POW_TAG_TYPE_ATOMIC) ?				\
	  "ATOMIC" :							\
	  (((x) == CVMX_POW_TAG_TYPE_NULL) ? "NULL" : "NULL_NULL")))

/* Error levels in WQE WORD2 (ERRLEV).*/
#define PKI_ERRLEV_E__RE_M 0x0
#define PKI_ERRLEV_E__LA_M 0x1
#define PKI_ERRLEV_E__LB_M 0x2
#define PKI_ERRLEV_E__LC_M 0x3
#define PKI_ERRLEV_E__LD_M 0x4
#define PKI_ERRLEV_E__LE_M 0x5
#define PKI_ERRLEV_E__LF_M 0x6
#define PKI_ERRLEV_E__LG_M 0x7

enum cvmx_pki_errlevel {
	CVMX_PKI_ERRLEV_E_RE = PKI_ERRLEV_E__RE_M,
	CVMX_PKI_ERRLEV_E_LA = PKI_ERRLEV_E__LA_M,
	CVMX_PKI_ERRLEV_E_LB = PKI_ERRLEV_E__LB_M,
	CVMX_PKI_ERRLEV_E_LC = PKI_ERRLEV_E__LC_M,
	CVMX_PKI_ERRLEV_E_LD = PKI_ERRLEV_E__LD_M,
	CVMX_PKI_ERRLEV_E_LE = PKI_ERRLEV_E__LE_M,
	CVMX_PKI_ERRLEV_E_LF = PKI_ERRLEV_E__LF_M,
	CVMX_PKI_ERRLEV_E_LG = PKI_ERRLEV_E__LG_M
};

#define CVMX_PKI_ERRLEV_MAX BIT(3) /* The size of WORD2:ERRLEV field.*/

/* Error code in WQE WORD2 (OPCODE).*/
#define CVMX_PKI_OPCODE_RE_NONE	      0x0
#define CVMX_PKI_OPCODE_RE_PARTIAL    0x1
#define CVMX_PKI_OPCODE_RE_JABBER     0x2
#define CVMX_PKI_OPCODE_RE_FCS	      0x7
#define CVMX_PKI_OPCODE_RE_FCS_RCV    0x8
#define CVMX_PKI_OPCODE_RE_TERMINATE  0x9
#define CVMX_PKI_OPCODE_RE_RX_CTL     0xb
#define CVMX_PKI_OPCODE_RE_SKIP	      0xc
#define CVMX_PKI_OPCODE_RE_DMAPKT     0xf
#define CVMX_PKI_OPCODE_RE_PKIPAR     0x13
#define CVMX_PKI_OPCODE_RE_PKIPCAM    0x14
#define CVMX_PKI_OPCODE_RE_MEMOUT     0x15
#define CVMX_PKI_OPCODE_RE_BUFS_OFLOW 0x16
#define CVMX_PKI_OPCODE_L2_FRAGMENT   0x20
#define CVMX_PKI_OPCODE_L2_OVERRUN    0x21
#define CVMX_PKI_OPCODE_L2_PFCS	      0x22
#define CVMX_PKI_OPCODE_L2_PUNY	      0x23
#define CVMX_PKI_OPCODE_L2_MAL	      0x24
#define CVMX_PKI_OPCODE_L2_OVERSIZE   0x25
#define CVMX_PKI_OPCODE_L2_UNDERSIZE  0x26
#define CVMX_PKI_OPCODE_L2_LENMISM    0x27
#define CVMX_PKI_OPCODE_IP_NOT	      0x41
#define CVMX_PKI_OPCODE_IP_CHK	      0x42
#define CVMX_PKI_OPCODE_IP_MAL	      0x43
#define CVMX_PKI_OPCODE_IP_MALD	      0x44
#define CVMX_PKI_OPCODE_IP_HOP	      0x45
#define CVMX_PKI_OPCODE_L4_MAL	      0x61
#define CVMX_PKI_OPCODE_L4_CHK	      0x62
#define CVMX_PKI_OPCODE_L4_LEN	      0x63
#define CVMX_PKI_OPCODE_L4_PORT	      0x64
#define CVMX_PKI_OPCODE_TCP_FLAG      0x65

#define CVMX_PKI_OPCODE_MAX BIT(8) /* The size of WORD2:OPCODE field.*/

/* Layer types in pki */
#define CVMX_PKI_LTYPE_E_NONE_M	      0x0
#define CVMX_PKI_LTYPE_E_ENET_M	      0x1
#define CVMX_PKI_LTYPE_E_VLAN_M	      0x2
#define CVMX_PKI_LTYPE_E_SNAP_PAYLD_M 0x5
#define CVMX_PKI_LTYPE_E_ARP_M	      0x6
#define CVMX_PKI_LTYPE_E_RARP_M	      0x7
#define CVMX_PKI_LTYPE_E_IP4_M	      0x8
#define CVMX_PKI_LTYPE_E_IP4_OPT_M    0x9
#define CVMX_PKI_LTYPE_E_IP6_M	      0xA
#define CVMX_PKI_LTYPE_E_IP6_OPT_M    0xB
#define CVMX_PKI_LTYPE_E_IPSEC_ESP_M  0xC
#define CVMX_PKI_LTYPE_E_IPFRAG_M     0xD
#define CVMX_PKI_LTYPE_E_IPCOMP_M     0xE
#define CVMX_PKI_LTYPE_E_TCP_M	      0x10
#define CVMX_PKI_LTYPE_E_UDP_M	      0x11
#define CVMX_PKI_LTYPE_E_SCTP_M	      0x12
#define CVMX_PKI_LTYPE_E_UDP_VXLAN_M  0x13
#define CVMX_PKI_LTYPE_E_GRE_M	      0x14
#define CVMX_PKI_LTYPE_E_NVGRE_M      0x15
#define CVMX_PKI_LTYPE_E_GTP_M	      0x16
#define CVMX_PKI_LTYPE_E_SW28_M	      0x1C
#define CVMX_PKI_LTYPE_E_SW29_M	      0x1D
#define CVMX_PKI_LTYPE_E_SW30_M	      0x1E
#define CVMX_PKI_LTYPE_E_SW31_M	      0x1F

enum cvmx_pki_layer_type {
	CVMX_PKI_LTYPE_E_NONE = CVMX_PKI_LTYPE_E_NONE_M,
	CVMX_PKI_LTYPE_E_ENET = CVMX_PKI_LTYPE_E_ENET_M,
	CVMX_PKI_LTYPE_E_VLAN = CVMX_PKI_LTYPE_E_VLAN_M,
	CVMX_PKI_LTYPE_E_SNAP_PAYLD = CVMX_PKI_LTYPE_E_SNAP_PAYLD_M,
	CVMX_PKI_LTYPE_E_ARP = CVMX_PKI_LTYPE_E_ARP_M,
	CVMX_PKI_LTYPE_E_RARP = CVMX_PKI_LTYPE_E_RARP_M,
	CVMX_PKI_LTYPE_E_IP4 = CVMX_PKI_LTYPE_E_IP4_M,
	CVMX_PKI_LTYPE_E_IP4_OPT = CVMX_PKI_LTYPE_E_IP4_OPT_M,
	CVMX_PKI_LTYPE_E_IP6 = CVMX_PKI_LTYPE_E_IP6_M,
	CVMX_PKI_LTYPE_E_IP6_OPT = CVMX_PKI_LTYPE_E_IP6_OPT_M,
	CVMX_PKI_LTYPE_E_IPSEC_ESP = CVMX_PKI_LTYPE_E_IPSEC_ESP_M,
	CVMX_PKI_LTYPE_E_IPFRAG = CVMX_PKI_LTYPE_E_IPFRAG_M,
	CVMX_PKI_LTYPE_E_IPCOMP = CVMX_PKI_LTYPE_E_IPCOMP_M,
	CVMX_PKI_LTYPE_E_TCP = CVMX_PKI_LTYPE_E_TCP_M,
	CVMX_PKI_LTYPE_E_UDP = CVMX_PKI_LTYPE_E_UDP_M,
	CVMX_PKI_LTYPE_E_SCTP = CVMX_PKI_LTYPE_E_SCTP_M,
	CVMX_PKI_LTYPE_E_UDP_VXLAN = CVMX_PKI_LTYPE_E_UDP_VXLAN_M,
	CVMX_PKI_LTYPE_E_GRE = CVMX_PKI_LTYPE_E_GRE_M,
	CVMX_PKI_LTYPE_E_NVGRE = CVMX_PKI_LTYPE_E_NVGRE_M,
	CVMX_PKI_LTYPE_E_GTP = CVMX_PKI_LTYPE_E_GTP_M,
	CVMX_PKI_LTYPE_E_SW28 = CVMX_PKI_LTYPE_E_SW28_M,
	CVMX_PKI_LTYPE_E_SW29 = CVMX_PKI_LTYPE_E_SW29_M,
	CVMX_PKI_LTYPE_E_SW30 = CVMX_PKI_LTYPE_E_SW30_M,
	CVMX_PKI_LTYPE_E_SW31 = CVMX_PKI_LTYPE_E_SW31_M,
	CVMX_PKI_LTYPE_E_MAX = CVMX_PKI_LTYPE_E_SW31
};

typedef union {
	u64 u64;
	struct {
		u64 ptr_vlan : 8;
		u64 ptr_layer_g : 8;
		u64 ptr_layer_f : 8;
		u64 ptr_layer_e : 8;
		u64 ptr_layer_d : 8;
		u64 ptr_layer_c : 8;
		u64 ptr_layer_b : 8;
		u64 ptr_layer_a : 8;
	};
} cvmx_pki_wqe_word4_t;

/**
 * HW decode / err_code in work queue entry
 */
typedef union {
	u64 u64;
	struct {
		u64 bufs : 8;
		u64 ip_offset : 8;
		u64 vlan_valid : 1;
		u64 vlan_stacked : 1;
		u64 unassigned : 1;
		u64 vlan_cfi : 1;
		u64 vlan_id : 12;
		u64 varies : 12;
		u64 dec_ipcomp : 1;
		u64 tcp_or_udp : 1;
		u64 dec_ipsec : 1;
		u64 is_v6 : 1;
		u64 software : 1;
		u64 L4_error : 1;
		u64 is_frag : 1;
		u64 IP_exc : 1;
		u64 is_bcast : 1;
		u64 is_mcast : 1;
		u64 not_IP : 1;
		u64 rcv_error : 1;
		u64 err_code : 8;
	} s;
	struct {
		u64 bufs : 8;
		u64 ip_offset : 8;
		u64 vlan_valid : 1;
		u64 vlan_stacked : 1;
		u64 unassigned : 1;
		u64 vlan_cfi : 1;
		u64 vlan_id : 12;
		u64 port : 12;
		u64 dec_ipcomp : 1;
		u64 tcp_or_udp : 1;
		u64 dec_ipsec : 1;
		u64 is_v6 : 1;
		u64 software : 1;
		u64 L4_error : 1;
		u64 is_frag : 1;
		u64 IP_exc : 1;
		u64 is_bcast : 1;
		u64 is_mcast : 1;
		u64 not_IP : 1;
		u64 rcv_error : 1;
		u64 err_code : 8;
	} s_cn68xx;
	struct {
		u64 bufs : 8;
		u64 ip_offset : 8;
		u64 vlan_valid : 1;
		u64 vlan_stacked : 1;
		u64 unassigned : 1;
		u64 vlan_cfi : 1;
		u64 vlan_id : 12;
		u64 pr : 4;
		u64 unassigned2a : 4;
		u64 unassigned2 : 4;
		u64 dec_ipcomp : 1;
		u64 tcp_or_udp : 1;
		u64 dec_ipsec : 1;
		u64 is_v6 : 1;
		u64 software : 1;
		u64 L4_error : 1;
		u64 is_frag : 1;
		u64 IP_exc : 1;
		u64 is_bcast : 1;
		u64 is_mcast : 1;
		u64 not_IP : 1;
		u64 rcv_error : 1;
		u64 err_code : 8;
	} s_cn38xx;
	struct {
		u64 unused1 : 16;
		u64 vlan : 16;
		u64 unused2 : 32;
	} svlan;
	struct {
		u64 bufs : 8;
		u64 unused : 8;
		u64 vlan_valid : 1;
		u64 vlan_stacked : 1;
		u64 unassigned : 1;
		u64 vlan_cfi : 1;
		u64 vlan_id : 12;
		u64 varies : 12;
		u64 unassigned2 : 4;
		u64 software : 1;
		u64 unassigned3 : 1;
		u64 is_rarp : 1;
		u64 is_arp : 1;
		u64 is_bcast : 1;
		u64 is_mcast : 1;
		u64 not_IP : 1;
		u64 rcv_error : 1;
		u64 err_code : 8;
	} snoip;
	struct {
		u64 bufs : 8;
		u64 unused : 8;
		u64 vlan_valid : 1;
		u64 vlan_stacked : 1;
		u64 unassigned : 1;
		u64 vlan_cfi : 1;
		u64 vlan_id : 12;
		u64 port : 12;
		u64 unassigned2 : 4;
		u64 software : 1;
		u64 unassigned3 : 1;
		u64 is_rarp : 1;
		u64 is_arp : 1;
		u64 is_bcast : 1;
		u64 is_mcast : 1;
		u64 not_IP : 1;
		u64 rcv_error : 1;
		u64 err_code : 8;
	} snoip_cn68xx;
	struct {
		u64 bufs : 8;
		u64 unused : 8;
		u64 vlan_valid : 1;
		u64 vlan_stacked : 1;
		u64 unassigned : 1;
		u64 vlan_cfi : 1;
		u64 vlan_id : 12;
		u64 pr : 4;
		u64 unassigned2a : 8;
		u64 unassigned2 : 4;
		u64 software : 1;
		u64 unassigned3 : 1;
		u64 is_rarp : 1;
		u64 is_arp : 1;
		u64 is_bcast : 1;
		u64 is_mcast : 1;
		u64 not_IP : 1;
		u64 rcv_error : 1;
		u64 err_code : 8;
	} snoip_cn38xx;
} cvmx_pip_wqe_word2_t;

typedef union {
	u64 u64;
	struct {
		u64 software : 1;
		u64 lg_hdr_type : 5;
		u64 lf_hdr_type : 5;
		u64 le_hdr_type : 5;
		u64 ld_hdr_type : 5;
		u64 lc_hdr_type : 5;
		u64 lb_hdr_type : 5;
		u64 is_la_ether : 1;
		u64 rsvd_0 : 8;
		u64 vlan_valid : 1;
		u64 vlan_stacked : 1;
		u64 stat_inc : 1;
		u64 pcam_flag4 : 1;
		u64 pcam_flag3 : 1;
		u64 pcam_flag2 : 1;
		u64 pcam_flag1 : 1;
		u64 is_frag : 1;
		u64 is_l3_bcast : 1;
		u64 is_l3_mcast : 1;
		u64 is_l2_bcast : 1;
		u64 is_l2_mcast : 1;
		u64 is_raw : 1;
		u64 err_level : 3;
		u64 err_code : 8;
	};
} cvmx_pki_wqe_word2_t;

typedef union {
	u64 u64;
	cvmx_pki_wqe_word2_t pki;
	cvmx_pip_wqe_word2_t pip;
} cvmx_wqe_word2_t;

typedef union {
	u64 u64;
	struct {
		u16 hw_chksum;
		u8 unused;
		u64 next_ptr : 40;
	} cn38xx;
	struct {
		u64 l4ptr : 8;	  /* 56..63 */
		u64 unused0 : 8;  /* 48..55 */
		u64 l3ptr : 8;	  /* 40..47 */
		u64 l2ptr : 8;	  /* 32..39 */
		u64 unused1 : 18; /* 14..31 */
		u64 bpid : 6;	  /* 8..13 */
		u64 unused2 : 2;  /* 6..7 */
		u64 pknd : 6;	  /* 0..5 */
	} cn68xx;
} cvmx_pip_wqe_word0_t;

typedef union {
	u64 u64;
	struct {
		u64 rsvd_0 : 4;
		u64 aura : 12;
		u64 rsvd_1 : 1;
		u64 apad : 3;
		u64 channel : 12;
		u64 bufs : 8;
		u64 style : 8;
		u64 rsvd_2 : 10;
		u64 pknd : 6;
	};
} cvmx_pki_wqe_word0_t;

/* Use reserved bit, set by HW to 0, to indicate buf_ptr legacy translation*/
#define pki_wqe_translated word0.rsvd_1

typedef union {
	u64 u64;
	cvmx_pip_wqe_word0_t pip;
	cvmx_pki_wqe_word0_t pki;
	struct {
		u64 unused : 24;
		u64 next_ptr : 40; /* On cn68xx this is unused as well */
	} raw;
} cvmx_wqe_word0_t;

typedef union {
	u64 u64;
	struct {
		u64 len : 16;
		u64 rsvd_0 : 2;
		u64 rsvd_1 : 2;
		u64 grp : 10;
		cvmx_pow_tag_type_t tag_type : 2;
		u64 tag : 32;
	};
} cvmx_pki_wqe_word1_t;

#define pki_errata20776 word1.rsvd_0

typedef union {
	u64 u64;
	struct {
		u64 len : 16;
		u64 varies : 14;
		cvmx_pow_tag_type_t tag_type : 2;
		u64 tag : 32;
	};
	cvmx_pki_wqe_word1_t cn78xx;
	struct {
		u64 len : 16;
		u64 zero_0 : 1;
		u64 qos : 3;
		u64 zero_1 : 1;
		u64 grp : 6;
		u64 zero_2 : 3;
		cvmx_pow_tag_type_t tag_type : 2;
		u64 tag : 32;
	} cn68xx;
	struct {
		u64 len : 16;
		u64 ipprt : 6;
		u64 qos : 3;
		u64 grp : 4;
		u64 zero_2 : 1;
		cvmx_pow_tag_type_t tag_type : 2;
		u64 tag : 32;
	} cn38xx;
} cvmx_wqe_word1_t;

typedef union {
	u64 u64;
	struct {
		u64 rsvd_0 : 8;
		u64 hwerr : 8;
		u64 rsvd_1 : 24;
		u64 sqid : 8;
		u64 rsvd_2 : 4;
		u64 vfnum : 12;
	};
} cvmx_wqe_word3_t;

typedef union {
	u64 u64;
	struct {
		u64 rsvd_0 : 21;
		u64 sqfc : 11;
		u64 rsvd_1 : 5;
		u64 sqtail : 11;
		u64 rsvd_2 : 3;
		u64 sqhead : 13;
	};
} cvmx_wqe_word4_t;

/**
 * Work queue entry format.
 * Must be 8-byte aligned.
 */
typedef struct cvmx_wqe_s {
	/*-------------------------------------------------------------------*/
	/* WORD 0                                                            */
	/*-------------------------------------------------------------------*/
	/* HW WRITE: the following 64 bits are filled by HW when a packet
	 * arrives.
	 */
	cvmx_wqe_word0_t word0;

	/*-------------------------------------------------------------------*/
	/* WORD 1                                                            */
	/*-------------------------------------------------------------------*/
	/* HW WRITE: the following 64 bits are filled by HW when a packet
	 * arrives.
	 */
	cvmx_wqe_word1_t word1;

	/*-------------------------------------------------------------------*/
	/* WORD 2                                                            */
	/*-------------------------------------------------------------------*/
	/* HW WRITE: the following 64-bits are filled in by hardware when a
	 * packet arrives. This indicates a variety of status and error
	 *conditions.
	 */
	cvmx_pip_wqe_word2_t word2;

	/* Pointer to the first segment of the packet. */
	cvmx_buf_ptr_t packet_ptr;

	/* HW WRITE: OCTEON will fill in a programmable amount from the packet,
	 * up to (at most, but perhaps less) the amount needed to fill the work
	 * queue entry to 128 bytes. If the packet is recognized to be IP, the
	 * hardware starts (except that the IPv4 header is padded for
	 * appropriate alignment) writing here where the IP header starts.
	 * If the packet is not recognized to be IP, the hardware starts
	 * writing the beginning of the packet here.
	 */
	u8 packet_data[96];

	/* If desired, SW can make the work Q entry any length. For the purposes
	 * of discussion here, Assume 128B always, as this is all that the hardware
	 * deals with.
	 */
} CVMX_CACHE_LINE_ALIGNED cvmx_wqe_t;

/**
 * Work queue entry format for NQM
 * Must be 8-byte aligned
 */
typedef struct cvmx_wqe_nqm_s {
	/*-------------------------------------------------------------------*/
	/* WORD 0                                                            */
	/*-------------------------------------------------------------------*/
	/* HW WRITE: the following 64 bits are filled by HW when a packet
	 * arrives.
	 */
	cvmx_wqe_word0_t word0;

	/*-------------------------------------------------------------------*/
	/* WORD 1                                                            */
	/*-------------------------------------------------------------------*/
	/* HW WRITE: the following 64 bits are filled by HW when a packet
	 * arrives.
	 */
	cvmx_wqe_word1_t word1;

	/*-------------------------------------------------------------------*/
	/* WORD 2                                                            */
	/*-------------------------------------------------------------------*/
	/* Reserved */
	u64 word2;

	/*-------------------------------------------------------------------*/
	/* WORD 3                                                            */
	/*-------------------------------------------------------------------*/
	/* NVMe specific information.*/
	cvmx_wqe_word3_t word3;

	/*-------------------------------------------------------------------*/
	/* WORD 4                                                            */
	/*-------------------------------------------------------------------*/
	/* NVMe specific information.*/
	cvmx_wqe_word4_t word4;

	/* HW WRITE: OCTEON will fill in a programmable amount from the packet,
	 * up to (at most, but perhaps less) the amount needed to fill the work
	 * queue entry to 128 bytes. If the packet is recognized to be IP, the
	 * hardware starts (except that the IPv4 header is padded for
	 * appropriate alignment) writing here where the IP header starts.
	 * If the packet is not recognized to be IP, the hardware starts
	 * writing the beginning of the packet here.
	 */
	u8 packet_data[88];

	/* If desired, SW can make the work Q entry any length.
	 * For the purposes of discussion here, assume 128B always, as this is
	 * all that the hardware deals with.
	 */
} CVMX_CACHE_LINE_ALIGNED cvmx_wqe_nqm_t;

/**
 * Work queue entry format for 78XX.
 * In 78XX packet data always resides in WQE buffer unless option
 * DIS_WQ_DAT=1 in PKI_STYLE_BUF, which causes packet data to use separate buffer.
 *
 * Must be 8-byte aligned.
 */
typedef struct {
	/*-------------------------------------------------------------------*/
	/* WORD 0                                                            */
	/*-------------------------------------------------------------------*/
	/* HW WRITE: the following 64 bits are filled by HW when a packet
	 * arrives.
	 */
	cvmx_pki_wqe_word0_t word0;

	/*-------------------------------------------------------------------*/
	/* WORD 1                                                            */
	/*-------------------------------------------------------------------*/
	/* HW WRITE: the following 64 bits are filled by HW when a packet
	 * arrives.
	 */
	cvmx_pki_wqe_word1_t word1;

	/*-------------------------------------------------------------------*/
	/* WORD 2                                                            */
	/*-------------------------------------------------------------------*/
	/* HW WRITE: the following 64-bits are filled in by hardware when a
	 * packet arrives. This indicates a variety of status and error
	 * conditions.
	 */
	cvmx_pki_wqe_word2_t word2;

	/*-------------------------------------------------------------------*/
	/* WORD 3                                                            */
	/*-------------------------------------------------------------------*/
	/* Pointer to the first segment of the packet.*/
	cvmx_buf_ptr_pki_t packet_ptr;

	/*-------------------------------------------------------------------*/
	/* WORD 4                                                            */
	/*-------------------------------------------------------------------*/
	/* HW WRITE: the following 64-bits are filled in by hardware when a
	 * packet arrives contains a byte pointer to the start of Layer
	 * A/B/C/D/E/F/G relative of start of packet.
	 */
	cvmx_pki_wqe_word4_t word4;

	/*-------------------------------------------------------------------*/
	/* WORDs 5/6/7 may be extended there, if WQE_HSZ is set.             */
	/*-------------------------------------------------------------------*/
	u64 wqe_data[11];

} CVMX_CACHE_LINE_ALIGNED cvmx_wqe_78xx_t;

/* Node LS-bit position in the WQE[grp] or PKI_QPG_TBL[grp_ok].*/
#define CVMX_WQE_GRP_NODE_SHIFT 8

/*
 * This is an accessor function into the WQE that retreives the
 * ingress port number, which can also be used as a destination
 * port number for the same port.
 *
 * @param work - Work Queue Entrey pointer
 * @returns returns the normalized port number, also known as "ipd" port
 */
static inline int cvmx_wqe_get_port(cvmx_wqe_t *work)
{
	int port;

	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		/* In 78xx wqe entry has channel number not port*/
		port = work->word0.pki.channel;
		/* For BGX interfaces (0x800 - 0xdff) the 4 LSBs indicate
		 * the PFC channel, must be cleared to normalize to "ipd"
		 */
		if (port & 0x800)
			port &= 0xff0;
		/* Node number is in AURA field, make it part of port # */
		port |= (work->word0.pki.aura >> 10) << 12;
	} else if (octeon_has_feature(OCTEON_FEATURE_CN68XX_WQE)) {
		port = work->word2.s_cn68xx.port;
	} else {
		port = work->word1.cn38xx.ipprt;
	}

	return port;
}

static inline void cvmx_wqe_set_port(cvmx_wqe_t *work, int port)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE))
		work->word0.pki.channel = port;
	else if (octeon_has_feature(OCTEON_FEATURE_CN68XX_WQE))
		work->word2.s_cn68xx.port = port;
	else
		work->word1.cn38xx.ipprt = port;
}

static inline int cvmx_wqe_get_grp(cvmx_wqe_t *work)
{
	int grp;

	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE))
		/* legacy: GRP[0..2] :=QOS */
		grp = (0xff & work->word1.cn78xx.grp) >> 3;
	else if (octeon_has_feature(OCTEON_FEATURE_CN68XX_WQE))
		grp = work->word1.cn68xx.grp;
	else
		grp = work->word1.cn38xx.grp;

	return grp;
}

static inline void cvmx_wqe_set_xgrp(cvmx_wqe_t *work, int grp)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE))
		work->word1.cn78xx.grp = grp;
	else if (octeon_has_feature(OCTEON_FEATURE_CN68XX_WQE))
		work->word1.cn68xx.grp = grp;
	else
		work->word1.cn38xx.grp = grp;
}

static inline int cvmx_wqe_get_xgrp(cvmx_wqe_t *work)
{
	int grp;

	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE))
		grp = work->word1.cn78xx.grp;
	else if (octeon_has_feature(OCTEON_FEATURE_CN68XX_WQE))
		grp = work->word1.cn68xx.grp;
	else
		grp = work->word1.cn38xx.grp;

	return grp;
}

static inline void cvmx_wqe_set_grp(cvmx_wqe_t *work, int grp)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		unsigned int node = cvmx_get_node_num();
		/* Legacy: GRP[0..2] :=QOS */
		work->word1.cn78xx.grp &= 0x7;
		work->word1.cn78xx.grp |= 0xff & (grp << 3);
		work->word1.cn78xx.grp |= (node << 8);
	} else if (octeon_has_feature(OCTEON_FEATURE_CN68XX_WQE)) {
		work->word1.cn68xx.grp = grp;
	} else {
		work->word1.cn38xx.grp = grp;
	}
}

static inline int cvmx_wqe_get_qos(cvmx_wqe_t *work)
{
	int qos;

	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		/* Legacy: GRP[0..2] :=QOS */
		qos = work->word1.cn78xx.grp & 0x7;
	} else if (octeon_has_feature(OCTEON_FEATURE_CN68XX_WQE)) {
		qos = work->word1.cn68xx.qos;
	} else {
		qos = work->word1.cn38xx.qos;
	}

	return qos;
}

static inline void cvmx_wqe_set_qos(cvmx_wqe_t *work, int qos)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		/* legacy: GRP[0..2] :=QOS */
		work->word1.cn78xx.grp &= ~0x7;
		work->word1.cn78xx.grp |= qos & 0x7;
	} else if (octeon_has_feature(OCTEON_FEATURE_CN68XX_WQE)) {
		work->word1.cn68xx.qos = qos;
	} else {
		work->word1.cn38xx.qos = qos;
	}
}

static inline int cvmx_wqe_get_len(cvmx_wqe_t *work)
{
	int len;

	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE))
		len = work->word1.cn78xx.len;
	else if (octeon_has_feature(OCTEON_FEATURE_CN68XX_WQE))
		len = work->word1.cn68xx.len;
	else
		len = work->word1.cn38xx.len;

	return len;
}

static inline void cvmx_wqe_set_len(cvmx_wqe_t *work, int len)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE))
		work->word1.cn78xx.len = len;
	else if (octeon_has_feature(OCTEON_FEATURE_CN68XX_WQE))
		work->word1.cn68xx.len = len;
	else
		work->word1.cn38xx.len = len;
}

/**
 * This function returns, if there was L2/L1 errors detected in packet.
 *
 * @param work	pointer to work queue entry
 *
 * Return:	0 if packet had no error, non-zero to indicate error code.
 *
 * Please refer to HRM for the specific model for full enumaration of error codes.
 * With Octeon1/Octeon2 models, the returned code indicates L1/L2 errors.
 * On CN73XX/CN78XX, the return code is the value of PKI_OPCODE_E,
 * if it is non-zero, otherwise the returned code will be derived from
 * PKI_ERRLEV_E such that an error indicated in LayerA will return 0x20,
 * LayerB - 0x30, LayerC - 0x40 and so forth.
 */
static inline int cvmx_wqe_get_rcv_err(cvmx_wqe_t *work)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		cvmx_wqe_78xx_t *wqe = (cvmx_wqe_78xx_t *)work;

		if (wqe->word2.err_level == CVMX_PKI_ERRLEV_E_RE || wqe->word2.err_code != 0)
			return wqe->word2.err_code;
		else
			return (wqe->word2.err_level << 4) + 0x10;
	} else if (work->word2.snoip.rcv_error) {
		return work->word2.snoip.err_code;
	}

	return 0;
}

static inline u32 cvmx_wqe_get_tag(cvmx_wqe_t *work)
{
	return work->word1.tag;
}

static inline void cvmx_wqe_set_tag(cvmx_wqe_t *work, u32 tag)
{
	work->word1.tag = tag;
}

static inline int cvmx_wqe_get_tt(cvmx_wqe_t *work)
{
	return work->word1.tag_type;
}

static inline void cvmx_wqe_set_tt(cvmx_wqe_t *work, int tt)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		work->word1.cn78xx.tag_type = (cvmx_pow_tag_type_t)tt;
	} else if (octeon_has_feature(OCTEON_FEATURE_CN68XX_WQE)) {
		work->word1.cn68xx.tag_type = (cvmx_pow_tag_type_t)tt;
		work->word1.cn68xx.zero_2 = 0;
	} else {
		work->word1.cn38xx.tag_type = (cvmx_pow_tag_type_t)tt;
		work->word1.cn38xx.zero_2 = 0;
	}
}

static inline u8 cvmx_wqe_get_unused8(cvmx_wqe_t *work)
{
	u8 bits;

	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		cvmx_wqe_78xx_t *wqe = (cvmx_wqe_78xx_t *)work;

		bits = wqe->word2.rsvd_0;
	} else if (octeon_has_feature(OCTEON_FEATURE_CN68XX_WQE)) {
		bits = work->word0.pip.cn68xx.unused1;
	} else {
		bits = work->word0.pip.cn38xx.unused;
	}

	return bits;
}

static inline void cvmx_wqe_set_unused8(cvmx_wqe_t *work, u8 v)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		cvmx_wqe_78xx_t *wqe = (cvmx_wqe_78xx_t *)work;

		wqe->word2.rsvd_0 = v;
	} else if (octeon_has_feature(OCTEON_FEATURE_CN68XX_WQE)) {
		work->word0.pip.cn68xx.unused1 = v;
	} else {
		work->word0.pip.cn38xx.unused = v;
	}
}

static inline u8 cvmx_wqe_get_user_flags(cvmx_wqe_t *work)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE))
		return work->word0.pki.rsvd_2;
	else
		return 0;
}

static inline void cvmx_wqe_set_user_flags(cvmx_wqe_t *work, u8 v)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE))
		work->word0.pki.rsvd_2 = v;
}

static inline int cvmx_wqe_get_channel(cvmx_wqe_t *work)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE))
		return (work->word0.pki.channel);
	else
		return cvmx_wqe_get_port(work);
}

static inline void cvmx_wqe_set_channel(cvmx_wqe_t *work, int channel)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE))
		work->word0.pki.channel = channel;
	else
		debug("%s: ERROR: not supported for model\n", __func__);
}

static inline int cvmx_wqe_get_aura(cvmx_wqe_t *work)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE))
		return (work->word0.pki.aura);
	else
		return (work->packet_ptr.s.pool);
}

static inline void cvmx_wqe_set_aura(cvmx_wqe_t *work, int aura)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE))
		work->word0.pki.aura = aura;
	else
		work->packet_ptr.s.pool = aura;
}

static inline int cvmx_wqe_get_style(cvmx_wqe_t *work)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE))
		return (work->word0.pki.style);
	return 0;
}

static inline void cvmx_wqe_set_style(cvmx_wqe_t *work, int style)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE))
		work->word0.pki.style = style;
}

static inline int cvmx_wqe_is_l3_ip(cvmx_wqe_t *work)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		cvmx_wqe_78xx_t *wqe = (cvmx_wqe_78xx_t *)work;
		/* Match all 4 values for v4/v6 with.without options */
		if ((wqe->word2.lc_hdr_type & 0x1c) == CVMX_PKI_LTYPE_E_IP4)
			return 1;
		if ((wqe->word2.le_hdr_type & 0x1c) == CVMX_PKI_LTYPE_E_IP4)
			return 1;
		return 0;
	} else {
		return !work->word2.s_cn38xx.not_IP;
	}
}

static inline int cvmx_wqe_is_l3_ipv4(cvmx_wqe_t *work)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		cvmx_wqe_78xx_t *wqe = (cvmx_wqe_78xx_t *)work;
		/* Match 2 values - with/wotuout options */
		if ((wqe->word2.lc_hdr_type & 0x1e) == CVMX_PKI_LTYPE_E_IP4)
			return 1;
		if ((wqe->word2.le_hdr_type & 0x1e) == CVMX_PKI_LTYPE_E_IP4)
			return 1;
		return 0;
	} else {
		return (!work->word2.s_cn38xx.not_IP &&
			!work->word2.s_cn38xx.is_v6);
	}
}

static inline int cvmx_wqe_is_l3_ipv6(cvmx_wqe_t *work)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		cvmx_wqe_78xx_t *wqe = (cvmx_wqe_78xx_t *)work;
		/* Match 2 values - with/wotuout options */
		if ((wqe->word2.lc_hdr_type & 0x1e) == CVMX_PKI_LTYPE_E_IP6)
			return 1;
		if ((wqe->word2.le_hdr_type & 0x1e) == CVMX_PKI_LTYPE_E_IP6)
			return 1;
		return 0;
	} else {
		return (!work->word2.s_cn38xx.not_IP &&
			work->word2.s_cn38xx.is_v6);
	}
}

static inline bool cvmx_wqe_is_l4_udp_or_tcp(cvmx_wqe_t *work)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		cvmx_wqe_78xx_t *wqe = (cvmx_wqe_78xx_t *)work;

		if (wqe->word2.lf_hdr_type == CVMX_PKI_LTYPE_E_TCP)
			return true;
		if (wqe->word2.lf_hdr_type == CVMX_PKI_LTYPE_E_UDP)
			return true;
		return false;
	}

	if (work->word2.s_cn38xx.not_IP)
		return false;

	return (work->word2.s_cn38xx.tcp_or_udp != 0);
}

static inline int cvmx_wqe_is_l2_bcast(cvmx_wqe_t *work)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		cvmx_wqe_78xx_t *wqe = (cvmx_wqe_78xx_t *)work;

		return wqe->word2.is_l2_bcast;
	} else {
		return work->word2.s_cn38xx.is_bcast;
	}
}

static inline int cvmx_wqe_is_l2_mcast(cvmx_wqe_t *work)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		cvmx_wqe_78xx_t *wqe = (cvmx_wqe_78xx_t *)work;

		return wqe->word2.is_l2_mcast;
	} else {
		return work->word2.s_cn38xx.is_mcast;
	}
}

static inline void cvmx_wqe_set_l2_bcast(cvmx_wqe_t *work, bool bcast)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		cvmx_wqe_78xx_t *wqe = (cvmx_wqe_78xx_t *)work;

		wqe->word2.is_l2_bcast = bcast;
	} else {
		work->word2.s_cn38xx.is_bcast = bcast;
	}
}

static inline void cvmx_wqe_set_l2_mcast(cvmx_wqe_t *work, bool mcast)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		cvmx_wqe_78xx_t *wqe = (cvmx_wqe_78xx_t *)work;

		wqe->word2.is_l2_mcast = mcast;
	} else {
		work->word2.s_cn38xx.is_mcast = mcast;
	}
}

static inline int cvmx_wqe_is_l3_bcast(cvmx_wqe_t *work)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		cvmx_wqe_78xx_t *wqe = (cvmx_wqe_78xx_t *)work;

		return wqe->word2.is_l3_bcast;
	}
	debug("%s: ERROR: not supported for model\n", __func__);
	return 0;
}

static inline int cvmx_wqe_is_l3_mcast(cvmx_wqe_t *work)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		cvmx_wqe_78xx_t *wqe = (cvmx_wqe_78xx_t *)work;

		return wqe->word2.is_l3_mcast;
	}
	debug("%s: ERROR: not supported for model\n", __func__);
	return 0;
}

/**
 * This function returns is there was IP error detected in packet.
 * For 78XX it does not flag ipv4 options and ipv6 extensions.
 * For older chips if PIP_GBL_CTL was proviosned to flag ip4_otions and
 * ipv6 extension, it will be flag them.
 * @param work	pointer to work queue entry
 * Return:	1 -- If IP error was found in packet
 *          0 -- If no IP error was found in packet.
 */
static inline int cvmx_wqe_is_ip_exception(cvmx_wqe_t *work)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		cvmx_wqe_78xx_t *wqe = (cvmx_wqe_78xx_t *)work;

		if (wqe->word2.err_level == CVMX_PKI_ERRLEV_E_LC)
			return 1;
		else
			return 0;
	}

	return work->word2.s.IP_exc;
}

static inline int cvmx_wqe_is_l4_error(cvmx_wqe_t *work)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		cvmx_wqe_78xx_t *wqe = (cvmx_wqe_78xx_t *)work;

		if (wqe->word2.err_level == CVMX_PKI_ERRLEV_E_LF)
			return 1;
		else
			return 0;
	} else {
		return work->word2.s.L4_error;
	}
}

static inline void cvmx_wqe_set_vlan(cvmx_wqe_t *work, bool set)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		cvmx_wqe_78xx_t *wqe = (cvmx_wqe_78xx_t *)work;

		wqe->word2.vlan_valid = set;
	} else {
		work->word2.s.vlan_valid = set;
	}
}

static inline int cvmx_wqe_is_vlan(cvmx_wqe_t *work)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		cvmx_wqe_78xx_t *wqe = (cvmx_wqe_78xx_t *)work;

		return wqe->word2.vlan_valid;
	} else {
		return work->word2.s.vlan_valid;
	}
}

static inline int cvmx_wqe_is_vlan_stacked(cvmx_wqe_t *work)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		cvmx_wqe_78xx_t *wqe = (cvmx_wqe_78xx_t *)work;

		return wqe->word2.vlan_stacked;
	} else {
		return work->word2.s.vlan_stacked;
	}
}

/**
 * Extract packet data buffer pointer from work queue entry.
 *
 * Returns the legacy (Octeon1/Octeon2) buffer pointer structure
 * for the linked buffer list.
 * On CN78XX, the native buffer pointer structure is converted into
 * the legacy format.
 * The legacy buf_ptr is then stored in the WQE, and word0 reserved
 * field is set to indicate that the buffer pointers were translated.
 * If the packet data is only found inside the work queue entry,
 * a standard buffer pointer structure is created for it.
 */
cvmx_buf_ptr_t cvmx_wqe_get_packet_ptr(cvmx_wqe_t *work);

static inline int cvmx_wqe_get_bufs(cvmx_wqe_t *work)
{
	int bufs;

	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		bufs = work->word0.pki.bufs;
	} else {
		/* Adjust for packet-in-WQE cases */
		if (cvmx_unlikely(work->word2.s_cn38xx.bufs == 0 && !work->word2.s.software))
			(void)cvmx_wqe_get_packet_ptr(work);
		bufs = work->word2.s_cn38xx.bufs;
	}
	return bufs;
}

/**
 * Free Work Queue Entry memory
 *
 * Will return the WQE buffer to its pool, unless the WQE contains
 * non-redundant packet data.
 * This function is intended to be called AFTER the packet data
 * has been passed along to PKO for transmission and release.
 * It can also follow a call to cvmx_helper_free_packet_data()
 * to release the WQE after associated data was released.
 */
void cvmx_wqe_free(cvmx_wqe_t *work);

/**
 * Check if a work entry has been intiated by software
 *
 */
static inline bool cvmx_wqe_is_soft(cvmx_wqe_t *work)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		cvmx_wqe_78xx_t *wqe = (cvmx_wqe_78xx_t *)work;

		return wqe->word2.software;
	} else {
		return work->word2.s.software;
	}
}

/**
 * Allocate a work-queue entry for delivering software-initiated
 * event notifications.
 * The application data is copied into the work-queue entry,
 * if the space is sufficient.
 */
cvmx_wqe_t *cvmx_wqe_soft_create(void *data_p, unsigned int data_sz);

/* Errata (PKI-20776) PKI_BUFLINK_S's are endian-swapped
 * CN78XX pass 1.x has a bug where the packet pointer in each segment is
 * written in the opposite endianness of the configured mode. Fix these here.
 */
static inline void cvmx_wqe_pki_errata_20776(cvmx_wqe_t *work)
{
	cvmx_wqe_78xx_t *wqe = (cvmx_wqe_78xx_t *)work;

	if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && !wqe->pki_errata20776) {
		u64 bufs;
		cvmx_buf_ptr_pki_t buffer_next;

		bufs = wqe->word0.bufs;
		buffer_next = wqe->packet_ptr;
		while (bufs > 1) {
			cvmx_buf_ptr_pki_t next;
			void *nextaddr = cvmx_phys_to_ptr(buffer_next.addr - 8);

			memcpy(&next, nextaddr, sizeof(next));
			next.u64 = __builtin_bswap64(next.u64);
			memcpy(nextaddr, &next, sizeof(next));
			buffer_next = next;
			bufs--;
		}
		wqe->pki_errata20776 = 1;
	}
}

/**
 * @INTERNAL
 *
 * Extract the native PKI-specific buffer pointer from WQE.
 *
 * NOTE: Provisional, may be superceded.
 */
static inline cvmx_buf_ptr_pki_t cvmx_wqe_get_pki_pkt_ptr(cvmx_wqe_t *work)
{
	cvmx_wqe_78xx_t *wqe = (cvmx_wqe_78xx_t *)work;

	if (!octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		cvmx_buf_ptr_pki_t x = { 0 };
		return x;
	}

	cvmx_wqe_pki_errata_20776(work);
	return wqe->packet_ptr;
}

/**
 * Set the buffer segment count for a packet.
 *
 * Return: Returns the actual resulting value in the WQE fielda
 *
 */
static inline unsigned int cvmx_wqe_set_bufs(cvmx_wqe_t *work, unsigned int bufs)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		work->word0.pki.bufs = bufs;
		return work->word0.pki.bufs;
	}

	work->word2.s.bufs = bufs;
	return work->word2.s.bufs;
}

/**
 * Get the offset of Layer-3 header,
 * only supported when Layer-3 protocol is IPv4 or IPv6.
 *
 * Return: Returns the offset, or 0 if the offset is not known or unsupported.
 *
 * FIXME: Assuming word4 is present.
 */
static inline unsigned int cvmx_wqe_get_l3_offset(cvmx_wqe_t *work)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		cvmx_wqe_78xx_t *wqe = (cvmx_wqe_78xx_t *)work;
		/* Match 4 values: IPv4/v6 w/wo options */
		if ((wqe->word2.lc_hdr_type & 0x1c) == CVMX_PKI_LTYPE_E_IP4)
			return wqe->word4.ptr_layer_c;
	} else {
		return work->word2.s.ip_offset;
	}

	return 0;
}

/**
 * Set the offset of Layer-3 header in a packet.
 * Typically used when an IP packet is generated by software
 * or when the Layer-2 header length is modified, and
 * a subsequent recalculation of checksums is anticipated.
 *
 * Return: Returns the actual value of the work entry offset field.
 *
 * FIXME: Assuming word4 is present.
 */
static inline unsigned int cvmx_wqe_set_l3_offset(cvmx_wqe_t *work, unsigned int ip_off)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		cvmx_wqe_78xx_t *wqe = (cvmx_wqe_78xx_t *)work;
		/* Match 4 values: IPv4/v6 w/wo options */
		if ((wqe->word2.lc_hdr_type & 0x1c) == CVMX_PKI_LTYPE_E_IP4)
			wqe->word4.ptr_layer_c = ip_off;
	} else {
		work->word2.s.ip_offset = ip_off;
	}

	return cvmx_wqe_get_l3_offset(work);
}

/**
 * Set the indication that the packet contains a IPv4 Layer-3 * header.
 * Use 'cvmx_wqe_set_l3_ipv6()' if the protocol is IPv6.
 * When 'set' is false, the call will result in an indication
 * that the Layer-3 protocol is neither IPv4 nor IPv6.
 *
 * FIXME: Add IPV4_OPT handling based on L3 header length.
 */
static inline void cvmx_wqe_set_l3_ipv4(cvmx_wqe_t *work, bool set)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		cvmx_wqe_78xx_t *wqe = (cvmx_wqe_78xx_t *)work;

		if (set)
			wqe->word2.lc_hdr_type = CVMX_PKI_LTYPE_E_IP4;
		else
			wqe->word2.lc_hdr_type = CVMX_PKI_LTYPE_E_NONE;
	} else {
		work->word2.s.not_IP = !set;
		if (set)
			work->word2.s_cn38xx.is_v6 = 0;
	}
}

/**
 * Set packet Layer-3 protocol to IPv6.
 *
 * FIXME: Add IPV6_OPT handling based on presence of extended headers.
 */
static inline void cvmx_wqe_set_l3_ipv6(cvmx_wqe_t *work, bool set)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		cvmx_wqe_78xx_t *wqe = (cvmx_wqe_78xx_t *)work;

		if (set)
			wqe->word2.lc_hdr_type = CVMX_PKI_LTYPE_E_IP6;
		else
			wqe->word2.lc_hdr_type = CVMX_PKI_LTYPE_E_NONE;
	} else {
		work->word2.s_cn38xx.not_IP = !set;
		if (set)
			work->word2.s_cn38xx.is_v6 = 1;
	}
}

/**
 * Set a packet Layer-4 protocol type to UDP.
 */
static inline void cvmx_wqe_set_l4_udp(cvmx_wqe_t *work, bool set)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		cvmx_wqe_78xx_t *wqe = (cvmx_wqe_78xx_t *)work;

		if (set)
			wqe->word2.lf_hdr_type = CVMX_PKI_LTYPE_E_UDP;
		else
			wqe->word2.lf_hdr_type = CVMX_PKI_LTYPE_E_NONE;
	} else {
		if (!work->word2.s_cn38xx.not_IP)
			work->word2.s_cn38xx.tcp_or_udp = set;
	}
}

/**
 * Set a packet Layer-4 protocol type to TCP.
 */
static inline void cvmx_wqe_set_l4_tcp(cvmx_wqe_t *work, bool set)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		cvmx_wqe_78xx_t *wqe = (cvmx_wqe_78xx_t *)work;

		if (set)
			wqe->word2.lf_hdr_type = CVMX_PKI_LTYPE_E_TCP;
		else
			wqe->word2.lf_hdr_type = CVMX_PKI_LTYPE_E_NONE;
	} else {
		if (!work->word2.s_cn38xx.not_IP)
			work->word2.s_cn38xx.tcp_or_udp = set;
	}
}

/**
 * Set the "software" flag in a work entry.
 */
static inline void cvmx_wqe_set_soft(cvmx_wqe_t *work, bool set)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		cvmx_wqe_78xx_t *wqe = (cvmx_wqe_78xx_t *)work;

		wqe->word2.software = set;
	} else {
		work->word2.s.software = set;
	}
}

/**
 * Return true if the packet is an IP fragment.
 */
static inline bool cvmx_wqe_is_l3_frag(cvmx_wqe_t *work)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		cvmx_wqe_78xx_t *wqe = (cvmx_wqe_78xx_t *)work;

		return (wqe->word2.is_frag != 0);
	}

	if (!work->word2.s_cn38xx.not_IP)
		return (work->word2.s.is_frag != 0);

	return false;
}

/**
 * Set the indicator that the packet is an fragmented IP packet.
 */
static inline void cvmx_wqe_set_l3_frag(cvmx_wqe_t *work, bool set)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		cvmx_wqe_78xx_t *wqe = (cvmx_wqe_78xx_t *)work;

		wqe->word2.is_frag = set;
	} else {
		if (!work->word2.s_cn38xx.not_IP)
			work->word2.s.is_frag = set;
	}
}

/**
 * Set the packet Layer-3 protocol to RARP.
 */
static inline void cvmx_wqe_set_l3_rarp(cvmx_wqe_t *work, bool set)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		cvmx_wqe_78xx_t *wqe = (cvmx_wqe_78xx_t *)work;

		if (set)
			wqe->word2.lc_hdr_type = CVMX_PKI_LTYPE_E_RARP;
		else
			wqe->word2.lc_hdr_type = CVMX_PKI_LTYPE_E_NONE;
	} else {
		work->word2.snoip.is_rarp = set;
	}
}

/**
 * Set the packet Layer-3 protocol to ARP.
 */
static inline void cvmx_wqe_set_l3_arp(cvmx_wqe_t *work, bool set)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		cvmx_wqe_78xx_t *wqe = (cvmx_wqe_78xx_t *)work;

		if (set)
			wqe->word2.lc_hdr_type = CVMX_PKI_LTYPE_E_ARP;
		else
			wqe->word2.lc_hdr_type = CVMX_PKI_LTYPE_E_NONE;
	} else {
		work->word2.snoip.is_arp = set;
	}
}

/**
 * Return true if the packet Layer-3 protocol is ARP.
 */
static inline bool cvmx_wqe_is_l3_arp(cvmx_wqe_t *work)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		cvmx_wqe_78xx_t *wqe = (cvmx_wqe_78xx_t *)work;

		return (wqe->word2.lc_hdr_type == CVMX_PKI_LTYPE_E_ARP);
	}

	if (work->word2.s_cn38xx.not_IP)
		return (work->word2.snoip.is_arp != 0);

	return false;
}

#endif /* __CVMX_WQE_H__ */
