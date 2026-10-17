/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * Interface to the hardware Packet Input Data unit.
 */

#ifndef __CVMX_PKI_H__
#define __CVMX_PKI_H__

#include "cvmx-fpa3.h"
#include "cvmx-helper-util.h"
#include "cvmx-helper-cfg.h"
#include "cvmx-error.h"

/* PKI AURA and BPID count are equal to FPA AURA count */
#define CVMX_PKI_NUM_AURA	       (cvmx_fpa3_num_auras())
#define CVMX_PKI_NUM_BPID	       (cvmx_fpa3_num_auras())
#define CVMX_PKI_NUM_SSO_GROUP	       (cvmx_sso_num_xgrp())
#define CVMX_PKI_NUM_CLUSTER_GROUP_MAX 1
#define CVMX_PKI_NUM_CLUSTER_GROUP     (cvmx_pki_num_cl_grp())
#define CVMX_PKI_NUM_CLUSTER	       (cvmx_pki_num_clusters())

/* FIXME: Reduce some of these values, convert to routines XXX */
#define CVMX_PKI_NUM_CHANNEL	    4096
#define CVMX_PKI_NUM_PKIND	    64
#define CVMX_PKI_NUM_INTERNAL_STYLE 256
#define CVMX_PKI_NUM_FINAL_STYLE    64
#define CVMX_PKI_NUM_QPG_ENTRY	    2048
#define CVMX_PKI_NUM_MTAG_IDX	    (32 / 4) /* 32 registers grouped by 4*/
#define CVMX_PKI_NUM_LTYPE	    32
#define CVMX_PKI_NUM_PCAM_BANK	    2
#define CVMX_PKI_NUM_PCAM_ENTRY	    192
#define CVMX_PKI_NUM_FRAME_CHECK    2
#define CVMX_PKI_NUM_BELTYPE	    32
#define CVMX_PKI_MAX_FRAME_SIZE	    65535
#define CVMX_PKI_FIND_AVAL_ENTRY    (-1)
#define CVMX_PKI_CLUSTER_ALL	    0xf

#ifdef CVMX_SUPPORT_SEPARATE_CLUSTER_CONFIG
#define CVMX_PKI_TOTAL_PCAM_ENTRY                                                                  \
	((CVMX_PKI_NUM_CLUSTER) * (CVMX_PKI_NUM_PCAM_BANK) * (CVMX_PKI_NUM_PCAM_ENTRY))
#else
#define CVMX_PKI_TOTAL_PCAM_ENTRY (CVMX_PKI_NUM_PCAM_BANK * CVMX_PKI_NUM_PCAM_ENTRY)
#endif

static inline unsigned int cvmx_pki_num_clusters(void)
{
	if (OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX))
		return 2;
	return 4;
}

static inline unsigned int cvmx_pki_num_cl_grp(void)
{
	if (OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX) ||
	    OCTEON_IS_MODEL(OCTEON_CN78XX))
		return 1;
	return 0;
}

enum cvmx_pki_pkind_parse_mode {
	CVMX_PKI_PARSE_LA_TO_LG = 0,  /* Parse LA(L2) to LG */
	CVMX_PKI_PARSE_LB_TO_LG = 1,  /* Parse LB(custom) to LG */
	CVMX_PKI_PARSE_LC_TO_LG = 3,  /* Parse LC(L3) to LG */
	CVMX_PKI_PARSE_LG = 0x3f,     /* Parse LG */
	CVMX_PKI_PARSE_NOTHING = 0x7f /* Parse nothing */
};

enum cvmx_pki_parse_mode_chg {
	CVMX_PKI_PARSE_NO_CHG = 0x0,
	CVMX_PKI_PARSE_SKIP_TO_LB = 0x1,
	CVMX_PKI_PARSE_SKIP_TO_LC = 0x3,
	CVMX_PKI_PARSE_SKIP_TO_LD = 0x7,
	CVMX_PKI_PARSE_SKIP_TO_LG = 0x3f,
	CVMX_PKI_PARSE_SKIP_ALL = 0x7f,
};

enum cvmx_pki_l2_len_mode { PKI_L2_LENCHK_EQUAL_GREATER = 0, PKI_L2_LENCHK_EQUAL_ONLY };

enum cvmx_pki_cache_mode {
	CVMX_PKI_OPC_MODE_STT = 0LL,	  /* All blocks write through DRAM,*/
	CVMX_PKI_OPC_MODE_STF = 1LL,	  /* All blocks into L2 */
	CVMX_PKI_OPC_MODE_STF1_STT = 2LL, /* 1st block L2, rest DRAM */
	CVMX_PKI_OPC_MODE_STF2_STT = 3LL  /* 1st, 2nd blocks L2, rest DRAM */
};

/**
 * Tag type definitions
 */
enum cvmx_sso_tag_type {
	CVMX_SSO_TAG_TYPE_ORDERED = 0L,
	CVMX_SSO_TAG_TYPE_ATOMIC = 1L,
	CVMX_SSO_TAG_TYPE_UNTAGGED = 2L,
	CVMX_SSO_TAG_TYPE_EMPTY = 3L
};

enum cvmx_pki_qpg_qos {
	CVMX_PKI_QPG_QOS_NONE = 0,
	CVMX_PKI_QPG_QOS_VLAN,
	CVMX_PKI_QPG_QOS_MPLS,
	CVMX_PKI_QPG_QOS_DSA_SRC,
	CVMX_PKI_QPG_QOS_DIFFSERV,
	CVMX_PKI_QPG_QOS_HIGIG,
};

enum cvmx_pki_wqe_vlan { CVMX_PKI_USE_FIRST_VLAN = 0, CVMX_PKI_USE_SECOND_VLAN };

/**
 * Controls how the PKI statistics counters are handled
 * The PKI_STAT*_X registers can be indexed either by port kind (pkind), or
 * final style. (Does not apply to the PKI_STAT_INB* registers.)
 *    0 = X represents the packet’s pkind
 *    1 = X represents the low 6-bits of packet’s final style
 */
enum cvmx_pki_stats_mode { CVMX_PKI_STAT_MODE_PKIND, CVMX_PKI_STAT_MODE_STYLE };

enum cvmx_pki_fpa_wait { CVMX_PKI_DROP_PKT, CVMX_PKI_WAIT_PKT };

#define PKI_BELTYPE_E__NONE_M 0x0
#define PKI_BELTYPE_E__MISC_M 0x1
#define PKI_BELTYPE_E__IP4_M  0x2
#define PKI_BELTYPE_E__IP6_M  0x3
#define PKI_BELTYPE_E__TCP_M  0x4
#define PKI_BELTYPE_E__UDP_M  0x5
#define PKI_BELTYPE_E__SCTP_M 0x6
#define PKI_BELTYPE_E__SNAP_M 0x7

/* PKI_BELTYPE_E_t */
enum cvmx_pki_beltype {
	CVMX_PKI_BELTYPE_NONE = PKI_BELTYPE_E__NONE_M,
	CVMX_PKI_BELTYPE_MISC = PKI_BELTYPE_E__MISC_M,
	CVMX_PKI_BELTYPE_IP4 = PKI_BELTYPE_E__IP4_M,
	CVMX_PKI_BELTYPE_IP6 = PKI_BELTYPE_E__IP6_M,
	CVMX_PKI_BELTYPE_TCP = PKI_BELTYPE_E__TCP_M,
	CVMX_PKI_BELTYPE_UDP = PKI_BELTYPE_E__UDP_M,
	CVMX_PKI_BELTYPE_SCTP = PKI_BELTYPE_E__SCTP_M,
	CVMX_PKI_BELTYPE_SNAP = PKI_BELTYPE_E__SNAP_M,
	CVMX_PKI_BELTYPE_MAX = CVMX_PKI_BELTYPE_SNAP
};

struct cvmx_pki_frame_len {
	u16 maxlen;
	u16 minlen;
};

struct cvmx_pki_tag_fields {
	u64 layer_g_src : 1;
	u64 layer_f_src : 1;
	u64 layer_e_src : 1;
	u64 layer_d_src : 1;
	u64 layer_c_src : 1;
	u64 layer_b_src : 1;
	u64 layer_g_dst : 1;
	u64 layer_f_dst : 1;
	u64 layer_e_dst : 1;
	u64 layer_d_dst : 1;
	u64 layer_c_dst : 1;
	u64 layer_b_dst : 1;
	u64 input_port : 1;
	u64 mpls_label : 1;
	u64 first_vlan : 1;
	u64 second_vlan : 1;
	u64 ip_prot_nexthdr : 1;
	u64 tag_sync : 1;
	u64 tag_spi : 1;
	u64 tag_gtp : 1;
	u64 tag_vni : 1;
};

struct cvmx_pki_pkind_parse {
	u64 mpls_en : 1;
	u64 inst_hdr : 1;
	u64 lg_custom : 1;
	u64 fulc_en : 1;
	u64 dsa_en : 1;
	u64 hg2_en : 1;
	u64 hg_en : 1;
};

struct cvmx_pki_pool_config {
	int pool_num;
	cvmx_fpa3_pool_t pool;
	u64 buffer_size;
	u64 buffer_count;
};

struct cvmx_pki_qpg_config {
	int qpg_base;
	int port_add;
	int aura_num;
	int grp_ok;
	int grp_bad;
	int grptag_ok;
	int grptag_bad;
};

struct cvmx_pki_aura_config {
	int aura_num;
	int pool_num;
	cvmx_fpa3_pool_t pool;
	cvmx_fpa3_gaura_t aura;
	int buffer_count;
};

struct cvmx_pki_cluster_grp_config {
	int grp_num;
	u64 cluster_mask; /* Bit mask of cluster assigned to this cluster group */
};

struct cvmx_pki_sso_grp_config {
	int group;
	int priority;
	int weight;
	int affinity;
	u64 core_mask;
	u8 core_mask_set;
};

/* This is per style structure for configuring port parameters,
 * it is kind of of profile which can be assigned to any port.
 * If multiple ports are assigned same style be aware that modifying
 * that style will modify the respective parameters for all the ports
 * which are using this style
 */
struct cvmx_pki_style_parm {
	bool ip6_udp_opt;
	bool lenerr_en;
	bool maxerr_en;
	bool minerr_en;
	u8 lenerr_eqpad;
	u8 minmax_sel;
	bool qpg_dis_grptag;
	bool fcs_strip;
	bool fcs_chk;
	bool rawdrp;
	bool force_drop;
	bool nodrop;
	bool qpg_dis_padd;
	bool qpg_dis_grp;
	bool qpg_dis_aura;
	u16 qpg_base;
	enum cvmx_pki_qpg_qos qpg_qos;
	u8 qpg_port_sh;
	u8 qpg_port_msb;
	u8 apad_nip;
	u8 wqe_vs;
	enum cvmx_sso_tag_type tag_type;
	bool pkt_lend;
	u8 wqe_hsz;
	u16 wqe_skip;
	u16 first_skip;
	u16 later_skip;
	enum cvmx_pki_cache_mode cache_mode;
	u8 dis_wq_dat;
	u64 mbuff_size;
	bool len_lg;
	bool len_lf;
	bool len_le;
	bool len_ld;
	bool len_lc;
	bool len_lb;
	bool csum_lg;
	bool csum_lf;
	bool csum_le;
	bool csum_ld;
	bool csum_lc;
	bool csum_lb;
};

/* This is per style structure for configuring port's tag configuration,
 * it is kind of of profile which can be assigned to any port.
 * If multiple ports are assigned same style be aware that modiying that style
 * will modify the respective parameters for all the ports which are
 * using this style */
enum cvmx_pki_mtag_ptrsel {
	CVMX_PKI_MTAG_PTRSEL_SOP = 0,
	CVMX_PKI_MTAG_PTRSEL_LA = 8,
	CVMX_PKI_MTAG_PTRSEL_LB = 9,
	CVMX_PKI_MTAG_PTRSEL_LC = 10,
	CVMX_PKI_MTAG_PTRSEL_LD = 11,
	CVMX_PKI_MTAG_PTRSEL_LE = 12,
	CVMX_PKI_MTAG_PTRSEL_LF = 13,
	CVMX_PKI_MTAG_PTRSEL_LG = 14,
	CVMX_PKI_MTAG_PTRSEL_VL = 15,
};

struct cvmx_pki_mask_tag {
	bool enable;
	int base;   /* CVMX_PKI_MTAG_PTRSEL_XXX */
	int offset; /* Offset from base. */
	u64 val;    /* Bitmask:
		1 = enable, 0 = disabled for each byte in the 64-byte array.*/
};

struct cvmx_pki_style_tag_cfg {
	struct cvmx_pki_tag_fields tag_fields;
	struct cvmx_pki_mask_tag mask_tag[4];
};

struct cvmx_pki_style_config {
	struct cvmx_pki_style_parm parm_cfg;
	struct cvmx_pki_style_tag_cfg tag_cfg;
};

struct cvmx_pki_pkind_config {
	u8 cluster_grp;
	bool fcs_pres;
	struct cvmx_pki_pkind_parse parse_en;
	enum cvmx_pki_pkind_parse_mode initial_parse_mode;
	u8 fcs_skip;
	u8 inst_skip;
	int initial_style;
	bool custom_l2_hdr;
	u8 l2_scan_offset;
	u64 lg_scan_offset;
};

struct cvmx_pki_port_config {
	struct cvmx_pki_pkind_config pkind_cfg;
	struct cvmx_pki_style_config style_cfg;
};

struct cvmx_pki_global_parse {
	u64 virt_pen : 1;
	u64 clg_pen : 1;
	u64 cl2_pen : 1;
	u64 l4_pen : 1;
	u64 il3_pen : 1;
	u64 l3_pen : 1;
	u64 mpls_pen : 1;
	u64 fulc_pen : 1;
	u64 dsa_pen : 1;
	u64 hg_pen : 1;
};

struct cvmx_pki_tag_sec {
	u16 dst6;
	u16 src6;
	u16 dst;
	u16 src;
};

struct cvmx_pki_global_config {
	u64 cluster_mask[CVMX_PKI_NUM_CLUSTER_GROUP_MAX];
	enum cvmx_pki_stats_mode stat_mode;
	enum cvmx_pki_fpa_wait fpa_wait;
	struct cvmx_pki_global_parse gbl_pen;
	struct cvmx_pki_tag_sec tag_secret;
	struct cvmx_pki_frame_len frm_len[CVMX_PKI_NUM_FRAME_CHECK];
	enum cvmx_pki_beltype ltype_map[CVMX_PKI_NUM_BELTYPE];
	int pki_enable;
};

#define CVMX_PKI_PCAM_TERM_E_NONE_M	 0x0
#define CVMX_PKI_PCAM_TERM_E_L2_CUSTOM_M 0x2
#define CVMX_PKI_PCAM_TERM_E_HIGIGD_M	 0x4
#define CVMX_PKI_PCAM_TERM_E_HIGIG_M	 0x5
#define CVMX_PKI_PCAM_TERM_E_SMACH_M	 0x8
#define CVMX_PKI_PCAM_TERM_E_SMACL_M	 0x9
#define CVMX_PKI_PCAM_TERM_E_DMACH_M	 0xA
#define CVMX_PKI_PCAM_TERM_E_DMACL_M	 0xB
#define CVMX_PKI_PCAM_TERM_E_GLORT_M	 0x12
#define CVMX_PKI_PCAM_TERM_E_DSA_M	 0x13
#define CVMX_PKI_PCAM_TERM_E_ETHTYPE0_M	 0x18
#define CVMX_PKI_PCAM_TERM_E_ETHTYPE1_M	 0x19
#define CVMX_PKI_PCAM_TERM_E_ETHTYPE2_M	 0x1A
#define CVMX_PKI_PCAM_TERM_E_ETHTYPE3_M	 0x1B
#define CVMX_PKI_PCAM_TERM_E_MPLS0_M	 0x1E
#define CVMX_PKI_PCAM_TERM_E_L3_SIPHH_M	 0x1F
#define CVMX_PKI_PCAM_TERM_E_L3_SIPMH_M	 0x20
#define CVMX_PKI_PCAM_TERM_E_L3_SIPML_M	 0x21
#define CVMX_PKI_PCAM_TERM_E_L3_SIPLL_M	 0x22
#define CVMX_PKI_PCAM_TERM_E_L3_FLAGS_M	 0x23
#define CVMX_PKI_PCAM_TERM_E_L3_DIPHH_M	 0x24
#define CVMX_PKI_PCAM_TERM_E_L3_DIPMH_M	 0x25
#define CVMX_PKI_PCAM_TERM_E_L3_DIPML_M	 0x26
#define CVMX_PKI_PCAM_TERM_E_L3_DIPLL_M	 0x27
#define CVMX_PKI_PCAM_TERM_E_LD_VNI_M	 0x28
#define CVMX_PKI_PCAM_TERM_E_IL3_FLAGS_M 0x2B
#define CVMX_PKI_PCAM_TERM_E_LF_SPI_M	 0x2E
#define CVMX_PKI_PCAM_TERM_E_L4_SPORT_M	 0x2f
#define CVMX_PKI_PCAM_TERM_E_L4_PORT_M	 0x30
#define CVMX_PKI_PCAM_TERM_E_LG_CUSTOM_M 0x39

enum cvmx_pki_term {
	CVMX_PKI_PCAM_TERM_NONE = CVMX_PKI_PCAM_TERM_E_NONE_M,
	CVMX_PKI_PCAM_TERM_L2_CUSTOM = CVMX_PKI_PCAM_TERM_E_L2_CUSTOM_M,
	CVMX_PKI_PCAM_TERM_HIGIGD = CVMX_PKI_PCAM_TERM_E_HIGIGD_M,
	CVMX_PKI_PCAM_TERM_HIGIG = CVMX_PKI_PCAM_TERM_E_HIGIG_M,
	CVMX_PKI_PCAM_TERM_SMACH = CVMX_PKI_PCAM_TERM_E_SMACH_M,
	CVMX_PKI_PCAM_TERM_SMACL = CVMX_PKI_PCAM_TERM_E_SMACL_M,
	CVMX_PKI_PCAM_TERM_DMACH = CVMX_PKI_PCAM_TERM_E_DMACH_M,
	CVMX_PKI_PCAM_TERM_DMACL = CVMX_PKI_PCAM_TERM_E_DMACL_M,
	CVMX_PKI_PCAM_TERM_GLORT = CVMX_PKI_PCAM_TERM_E_GLORT_M,
	CVMX_PKI_PCAM_TERM_DSA = CVMX_PKI_PCAM_TERM_E_DSA_M,
	CVMX_PKI_PCAM_TERM_ETHTYPE0 = CVMX_PKI_PCAM_TERM_E_ETHTYPE0_M,
	CVMX_PKI_PCAM_TERM_ETHTYPE1 = CVMX_PKI_PCAM_TERM_E_ETHTYPE1_M,
	CVMX_PKI_PCAM_TERM_ETHTYPE2 = CVMX_PKI_PCAM_TERM_E_ETHTYPE2_M,
	CVMX_PKI_PCAM_TERM_ETHTYPE3 = CVMX_PKI_PCAM_TERM_E_ETHTYPE3_M,
	CVMX_PKI_PCAM_TERM_MPLS0 = CVMX_PKI_PCAM_TERM_E_MPLS0_M,
	CVMX_PKI_PCAM_TERM_L3_SIPHH = CVMX_PKI_PCAM_TERM_E_L3_SIPHH_M,
	CVMX_PKI_PCAM_TERM_L3_SIPMH = CVMX_PKI_PCAM_TERM_E_L3_SIPMH_M,
	CVMX_PKI_PCAM_TERM_L3_SIPML = CVMX_PKI_PCAM_TERM_E_L3_SIPML_M,
	CVMX_PKI_PCAM_TERM_L3_SIPLL = CVMX_PKI_PCAM_TERM_E_L3_SIPLL_M,
	CVMX_PKI_PCAM_TERM_L3_FLAGS = CVMX_PKI_PCAM_TERM_E_L3_FLAGS_M,
	CVMX_PKI_PCAM_TERM_L3_DIPHH = CVMX_PKI_PCAM_TERM_E_L3_DIPHH_M,
	CVMX_PKI_PCAM_TERM_L3_DIPMH = CVMX_PKI_PCAM_TERM_E_L3_DIPMH_M,
	CVMX_PKI_PCAM_TERM_L3_DIPML = CVMX_PKI_PCAM_TERM_E_L3_DIPML_M,
	CVMX_PKI_PCAM_TERM_L3_DIPLL = CVMX_PKI_PCAM_TERM_E_L3_DIPLL_M,
	CVMX_PKI_PCAM_TERM_LD_VNI = CVMX_PKI_PCAM_TERM_E_LD_VNI_M,
	CVMX_PKI_PCAM_TERM_IL3_FLAGS = CVMX_PKI_PCAM_TERM_E_IL3_FLAGS_M,
	CVMX_PKI_PCAM_TERM_LF_SPI = CVMX_PKI_PCAM_TERM_E_LF_SPI_M,
	CVMX_PKI_PCAM_TERM_L4_PORT = CVMX_PKI_PCAM_TERM_E_L4_PORT_M,
	CVMX_PKI_PCAM_TERM_L4_SPORT = CVMX_PKI_PCAM_TERM_E_L4_SPORT_M,
	CVMX_PKI_PCAM_TERM_LG_CUSTOM = CVMX_PKI_PCAM_TERM_E_LG_CUSTOM_M
};

#define CVMX_PKI_DMACH_SHIFT	  32
#define CVMX_PKI_DMACH_MASK	  cvmx_build_mask(16)
#define CVMX_PKI_DMACL_MASK	  CVMX_PKI_DATA_MASK_32
#define CVMX_PKI_DATA_MASK_32	  cvmx_build_mask(32)
#define CVMX_PKI_DATA_MASK_16	  cvmx_build_mask(16)
#define CVMX_PKI_DMAC_MATCH_EXACT cvmx_build_mask(48)

struct cvmx_pki_pcam_input {
	u64 style;
	u64 style_mask; /* bits: 1-match, 0-dont care */
	enum cvmx_pki_term field;
	u32 field_mask; /* bits: 1-match, 0-dont care */
	u64 data;
	u64 data_mask; /* bits: 1-match, 0-dont care */
};

struct cvmx_pki_pcam_action {
	enum cvmx_pki_parse_mode_chg parse_mode_chg;
	enum cvmx_pki_layer_type layer_type_set;
	int style_add;
	int parse_flag_set;
	int pointer_advance;
};

struct cvmx_pki_pcam_config {
	int in_use;
	int entry_num;
	u64 cluster_mask;
	struct cvmx_pki_pcam_input pcam_input;
	struct cvmx_pki_pcam_action pcam_action;
};

/**
 * Status statistics for a port
 */
struct cvmx_pki_port_stats {
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
};

/**
 * PKI Packet Instruction Header Structure (PKI_INST_HDR_S)
 */
typedef union {
	u64 u64;
	struct {
		u64 w : 1;    /* INST_HDR size: 0 = 2 bytes, 1 = 4 or 8 bytes */
		u64 raw : 1;  /* RAW packet indicator in WQE[RAW]: 1 = enable */
		u64 utag : 1; /* Use INST_HDR[TAG] to compute WQE[TAG]: 1 = enable */
		u64 uqpg : 1; /* Use INST_HDR[QPG] to compute QPG: 1 = enable */
		u64 rsvd1 : 1;
		u64 pm : 3; /* Packet parsing mode. Legal values = 0x0..0x7 */
		u64 sl : 8; /* Number of bytes in INST_HDR. */
		/* The following fields are not present, if INST_HDR[W] = 0: */
		u64 utt : 1; /* Use INST_HDR[TT] to compute WQE[TT]: 1 = enable */
		u64 tt : 2;  /* INST_HDR[TT] => WQE[TT], if INST_HDR[UTT] = 1 */
		u64 rsvd2 : 2;
		u64 qpg : 11; /* INST_HDR[QPG] => QPG, if INST_HDR[UQPG] = 1 */
		u64 tag : 32; /* INST_HDR[TAG] => WQE[TAG], if INST_HDR[UTAG] = 1 */
	} s;
} cvmx_pki_inst_hdr_t;

/**
 * This function assignes the clusters to a group, later pkind can be
 * configured to use that group depending on number of clusters pkind
 * would use. A given cluster can only be enabled in a single cluster group.
 * Number of clusters assign to that group determines how many engine can work
 * in parallel to process the packet. Eack cluster can process x MPPS.
 *
 * @param node	Node
 * @param cluster_group Group to attach clusters to.
 * @param cluster_mask The mask of clusters which needs to be assigned to the group.
 */
static inline int cvmx_pki_attach_cluster_to_group(int node, u64 cluster_group, u64 cluster_mask)
{
	cvmx_pki_icgx_cfg_t pki_cl_grp;

	if (cluster_group >= CVMX_PKI_NUM_CLUSTER_GROUP) {
		debug("ERROR: config cluster group %d", (int)cluster_group);
		return -1;
	}
	pki_cl_grp.u64 = cvmx_read_csr_node(node, CVMX_PKI_ICGX_CFG(cluster_group));
	pki_cl_grp.s.clusters = cluster_mask;
	cvmx_write_csr_node(node, CVMX_PKI_ICGX_CFG(cluster_group), pki_cl_grp.u64);
	return 0;
}

static inline void cvmx_pki_write_global_parse(int node, struct cvmx_pki_global_parse gbl_pen)
{
	cvmx_pki_gbl_pen_t gbl_pen_reg;

	gbl_pen_reg.u64 = cvmx_read_csr_node(node, CVMX_PKI_GBL_PEN);
	gbl_pen_reg.s.virt_pen = gbl_pen.virt_pen;
	gbl_pen_reg.s.clg_pen = gbl_pen.clg_pen;
	gbl_pen_reg.s.cl2_pen = gbl_pen.cl2_pen;
	gbl_pen_reg.s.l4_pen = gbl_pen.l4_pen;
	gbl_pen_reg.s.il3_pen = gbl_pen.il3_pen;
	gbl_pen_reg.s.l3_pen = gbl_pen.l3_pen;
	gbl_pen_reg.s.mpls_pen = gbl_pen.mpls_pen;
	gbl_pen_reg.s.fulc_pen = gbl_pen.fulc_pen;
	gbl_pen_reg.s.dsa_pen = gbl_pen.dsa_pen;
	gbl_pen_reg.s.hg_pen = gbl_pen.hg_pen;
	cvmx_write_csr_node(node, CVMX_PKI_GBL_PEN, gbl_pen_reg.u64);
}

static inline void cvmx_pki_write_tag_secret(int node, struct cvmx_pki_tag_sec tag_secret)
{
	cvmx_pki_tag_secret_t tag_secret_reg;

	tag_secret_reg.u64 = cvmx_read_csr_node(node, CVMX_PKI_TAG_SECRET);
	tag_secret_reg.s.dst6 = tag_secret.dst6;
	tag_secret_reg.s.src6 = tag_secret.src6;
	tag_secret_reg.s.dst = tag_secret.dst;
	tag_secret_reg.s.src = tag_secret.src;
	cvmx_write_csr_node(node, CVMX_PKI_TAG_SECRET, tag_secret_reg.u64);
}

static inline void cvmx_pki_write_ltype_map(int node, enum cvmx_pki_layer_type layer,
					    enum cvmx_pki_beltype backend)
{
	cvmx_pki_ltypex_map_t ltype_map;

	if (layer > CVMX_PKI_LTYPE_E_MAX || backend > CVMX_PKI_BELTYPE_MAX) {
		debug("ERROR: invalid ltype beltype mapping\n");
		return;
	}
	ltype_map.u64 = cvmx_read_csr_node(node, CVMX_PKI_LTYPEX_MAP(layer));
	ltype_map.s.beltype = backend;
	cvmx_write_csr_node(node, CVMX_PKI_LTYPEX_MAP(layer), ltype_map.u64);
}

/**
 * This function enables the cluster group to start parsing.
 *
 * @param node    Node number.
 * @param cl_grp  Cluster group to enable parsing.
 */
static inline int cvmx_pki_parse_enable(int node, unsigned int cl_grp)
{
	cvmx_pki_icgx_cfg_t pki_cl_grp;

	if (cl_grp >= CVMX_PKI_NUM_CLUSTER_GROUP) {
		debug("ERROR: pki parse en group %d", (int)cl_grp);
		return -1;
	}
	pki_cl_grp.u64 = cvmx_read_csr_node(node, CVMX_PKI_ICGX_CFG(cl_grp));
	pki_cl_grp.s.pena = 1;
	cvmx_write_csr_node(node, CVMX_PKI_ICGX_CFG(cl_grp), pki_cl_grp.u64);
	return 0;
}

/**
 * This function enables the PKI to send bpid level backpressure to CN78XX inputs.
 *
 * @param node Node number.
 */
static inline void cvmx_pki_enable_backpressure(int node)
{
	cvmx_pki_buf_ctl_t pki_buf_ctl;

	pki_buf_ctl.u64 = cvmx_read_csr_node(node, CVMX_PKI_BUF_CTL);
	pki_buf_ctl.s.pbp_en = 1;
	cvmx_write_csr_node(node, CVMX_PKI_BUF_CTL, pki_buf_ctl.u64);
}

/**
 * Clear the statistics counters for a port.
 *
 * @param node Node number.
 * @param port Port number (ipd_port) to get statistics for.
 *    Make sure PKI_STATS_CTL:mode is set to 0 for collecting per port/pkind stats.
 */
void cvmx_pki_clear_port_stats(int node, u64 port);

/**
 * Get the status counters for index from PKI.
 *
 * @param node	  Node number.
 * @param index   PKIND number, if PKI_STATS_CTL:mode = 0 or
 *     style(flow) number, if PKI_STATS_CTL:mode = 1
 * @param status  Where to put the results.
 */
void cvmx_pki_get_stats(int node, int index, struct cvmx_pki_port_stats *status);

/**
 * Get the statistics counters for a port.
 *
 * @param node	 Node number
 * @param port   Port number (ipd_port) to get statistics for.
 *    Make sure PKI_STATS_CTL:mode is set to 0 for collecting per port/pkind stats.
 * @param status Where to put the results.
 */
static inline void cvmx_pki_get_port_stats(int node, u64 port, struct cvmx_pki_port_stats *status)
{
	int xipd = cvmx_helper_node_to_ipd_port(node, port);
	int xiface = cvmx_helper_get_interface_num(xipd);
	int index = cvmx_helper_get_interface_index_num(port);
	int pknd = cvmx_helper_get_pknd(xiface, index);

	cvmx_pki_get_stats(node, pknd, status);
}

/**
 * Get the statistics counters for a flow represented by style in PKI.
 *
 * @param node Node number.
 * @param style_num Style number to get statistics for.
 *    Make sure PKI_STATS_CTL:mode is set to 1 for collecting per style/flow stats.
 * @param status Where to put the results.
 */
static inline void cvmx_pki_get_flow_stats(int node, u64 style_num,
					   struct cvmx_pki_port_stats *status)
{
	cvmx_pki_get_stats(node, style_num, status);
}

/**
 * Show integrated PKI configuration.
 *
 * @param node	   node number
 */
int cvmx_pki_config_dump(unsigned int node);

/**
 * Show integrated PKI statistics.
 *
 * @param node	   node number
 */
int cvmx_pki_stats_dump(unsigned int node);

/**
 * Clear PKI statistics.
 *
 * @param node	   node number
 */
void cvmx_pki_stats_clear(unsigned int node);

/**
 * This function enables PKI.
 *
 * @param node	 node to enable pki in.
 */
void cvmx_pki_enable(int node);

/**
 * This function disables PKI.
 *
 * @param node	node to disable pki in.
 */
void cvmx_pki_disable(int node);

/**
 * This function soft resets PKI.
 *
 * @param node	node to enable pki in.
 */
void cvmx_pki_reset(int node);

/**
 * This function sets the clusters in PKI.
 *
 * @param node	node to set clusters in.
 */
int cvmx_pki_setup_clusters(int node);

/**
 * This function reads global configuration of PKI block.
 *
 * @param node    Node number.
 * @param gbl_cfg Pointer to struct to read global configuration
 */
void cvmx_pki_read_global_config(int node, struct cvmx_pki_global_config *gbl_cfg);

/**
 * This function writes global configuration of PKI into hw.
 *
 * @param node    Node number.
 * @param gbl_cfg Pointer to struct to global configuration
 */
void cvmx_pki_write_global_config(int node, struct cvmx_pki_global_config *gbl_cfg);

/**
 * This function reads per pkind parameters in hardware which defines how
 * the incoming packet is processed.
 *
 * @param node   Node number.
 * @param pkind  PKI supports a large number of incoming interfaces and packets
 *     arriving on different interfaces or channels may want to be processed
 *     differently. PKI uses the pkind to determine how the incoming packet
 *     is processed.
 * @param pkind_cfg	Pointer to struct conatining pkind configuration read
 *     from hardware.
 */
int cvmx_pki_read_pkind_config(int node, int pkind, struct cvmx_pki_pkind_config *pkind_cfg);

/**
 * This function writes per pkind parameters in hardware which defines how
 * the incoming packet is processed.
 *
 * @param node   Node number.
 * @param pkind  PKI supports a large number of incoming interfaces and packets
 *     arriving on different interfaces or channels may want to be processed
 *     differently. PKI uses the pkind to determine how the incoming packet
 *     is processed.
 * @param pkind_cfg	Pointer to struct conatining pkind configuration need
 *     to be written in hardware.
 */
int cvmx_pki_write_pkind_config(int node, int pkind, struct cvmx_pki_pkind_config *pkind_cfg);

/**
 * This function reads parameters associated with tag configuration in hardware.
 *
 * @param node	 Node number.
 * @param style  Style to configure tag for.
 * @param cluster_mask  Mask of clusters to configure the style for.
 * @param tag_cfg  Pointer to tag configuration struct.
 */
void cvmx_pki_read_tag_config(int node, int style, u64 cluster_mask,
			      struct cvmx_pki_style_tag_cfg *tag_cfg);

/**
 * This function writes/configures parameters associated with tag
 * configuration in hardware.
 *
 * @param node  Node number.
 * @param style  Style to configure tag for.
 * @param cluster_mask  Mask of clusters to configure the style for.
 * @param tag_cfg  Pointer to taf configuration struct.
 */
void cvmx_pki_write_tag_config(int node, int style, u64 cluster_mask,
			       struct cvmx_pki_style_tag_cfg *tag_cfg);

/**
 * This function reads parameters associated with style in hardware.
 *
 * @param node	Node number.
 * @param style  Style to read from.
 * @param cluster_mask  Mask of clusters style belongs to.
 * @param style_cfg  Pointer to style config struct.
 */
void cvmx_pki_read_style_config(int node, int style, u64 cluster_mask,
				struct cvmx_pki_style_config *style_cfg);

/**
 * This function writes/configures parameters associated with style in hardware.
 *
 * @param node  Node number.
 * @param style  Style to configure.
 * @param cluster_mask  Mask of clusters to configure the style for.
 * @param style_cfg  Pointer to style config struct.
 */
void cvmx_pki_write_style_config(int node, u64 style, u64 cluster_mask,
				 struct cvmx_pki_style_config *style_cfg);
/**
 * This function reads qpg entry at specified offset from qpg table
 *
 * @param node  Node number.
 * @param offset  Offset in qpg table to read from.
 * @param qpg_cfg  Pointer to structure containing qpg values
 */
int cvmx_pki_read_qpg_entry(int node, int offset, struct cvmx_pki_qpg_config *qpg_cfg);

/**
 * This function writes qpg entry at specified offset in qpg table
 *
 * @param node  Node number.
 * @param offset  Offset in qpg table to write to.
 * @param qpg_cfg  Pointer to stricture containing qpg values.
 */
void cvmx_pki_write_qpg_entry(int node, int offset, struct cvmx_pki_qpg_config *qpg_cfg);

/**
 * This function writes pcam entry at given offset in pcam table in hardware
 *
 * @param node  Node number.
 * @param index	 Offset in pcam table.
 * @param cluster_mask  Mask of clusters in which to write pcam entry.
 * @param input  Input keys to pcam match passed as struct.
 * @param action  PCAM match action passed as struct
 */
int cvmx_pki_pcam_write_entry(int node, int index, u64 cluster_mask,
			      struct cvmx_pki_pcam_input input, struct cvmx_pki_pcam_action action);
/**
 * Configures the channel which will receive backpressure from the specified bpid.
 * Each channel listens for backpressure on a specific bpid.
 * Each bpid can backpressure multiple channels.
 * @param node  Node number.
 * @param bpid  BPID from which channel will receive backpressure.
 * @param channel  Channel number to receive backpressue.
 */
int cvmx_pki_write_channel_bpid(int node, int channel, int bpid);

/**
 * Configures the bpid on which, specified channel will
 * assert backpressure.
 * Each bpid receives backpressure from auras.
 * Multiple auras can backpressure single bpid.
 * @param node  Node number.
 * @param aura  Number which will assert backpressure on that bpid.
 * @param bpid  To assert backpressure on.
 */
int cvmx_pki_write_aura_bpid(int node, int aura, int bpid);

/**
 * Enables/Disabled QoS (RED Drop, Tail Drop & backpressure) for the* PKI aura.
 *
 * @param node  Node number
 * @param aura  To enable/disable QoS on.
 * @param ena_red  Enable/Disable RED drop between pass and drop level
 *    1-enable 0-disable
 * @param ena_drop  Enable/disable tail drop when max drop level exceeds
 *    1-enable 0-disable
 * @param ena_bp  Enable/Disable asserting backpressure on bpid when
 *    max DROP level exceeds.
 *    1-enable 0-disable
 */
int cvmx_pki_enable_aura_qos(int node, int aura, bool ena_red, bool ena_drop, bool ena_bp);

/**
 * This function gives the initial style used by that pkind.
 *
 * @param node  Node number.
 * @param pkind  PKIND number.
 */
int cvmx_pki_get_pkind_style(int node, int pkind);

/**
 * This function sets the wqe buffer mode. First packet data buffer can reside
 * either in same buffer as wqe OR it can go in separate buffer. If used the later mode,
 * make sure software allocate enough buffers to now have wqe separate from packet data.
 *
 * @param node  Node number.
 * @param style  Style to configure.
 * @param pkt_outside_wqe
 *    0 = The packet link pointer will be at word [FIRST_SKIP] immediately
 *    followed by packet data, in the same buffer as the work queue entry.
 *    1 = The packet link pointer will be at word [FIRST_SKIP] in a new
 *    buffer separate from the work queue entry. Words following the
 *    WQE in the same cache line will be zeroed, other lines in the
 *    buffer will not be modified and will retain stale data (from the
 *    buffer’s previous use). This setting may decrease the peak PKI
 *    performance by up to half on small packets.
 */
void cvmx_pki_set_wqe_mode(int node, u64 style, bool pkt_outside_wqe);

/**
 * This function sets the Packet mode of all ports and styles to little-endian.
 * It Changes write operations of packet data to L2C to
 * be in little-endian. Does not change the WQE header format, which is
 * properly endian neutral.
 *
 * @param node  Node number.
 * @param style  Style to configure.
 */
void cvmx_pki_set_little_endian(int node, u64 style);

/**
 * Enables/Disables L2 length error check and max & min frame length checks.
 *
 * @param node  Node number.
 * @param pknd  PKIND to disable error for.
 * @param l2len_err	 L2 length error check enable.
 * @param maxframe_err	Max frame error check enable.
 * @param minframe_err	Min frame error check enable.
 *    1 -- Enabel err checks
 *    0 -- Disable error checks
 */
void cvmx_pki_endis_l2_errs(int node, int pknd, bool l2len_err, bool maxframe_err,
			    bool minframe_err);

/**
 * Enables/Disables fcs check and fcs stripping on the pkind.
 *
 * @param node  Node number.
 * @param pknd  PKIND to apply settings on.
 * @param fcs_chk  Enable/disable fcs check.
 *    1 -- enable fcs error check.
 *    0 -- disable fcs error check.
 * @param fcs_strip	 Strip L2 FCS bytes from packet, decrease WQE[LEN] by 4 bytes
 *    1 -- strip L2 FCS.
 *    0 -- Do not strip L2 FCS.
 */
void cvmx_pki_endis_fcs_check(int node, int pknd, bool fcs_chk, bool fcs_strip);

/**
 * This function shows the qpg table entries, read directly from hardware.
 *
 * @param node  Node number.
 * @param num_entry  Number of entries to print.
 */
void cvmx_pki_show_qpg_entries(int node, u16 num_entry);

/**
 * This function shows the pcam table in raw format read directly from hardware.
 *
 * @param node  Node number.
 */
void cvmx_pki_show_pcam_entries(int node);

/**
 * This function shows the valid entries in readable format,
 * read directly from hardware.
 *
 * @param node  Node number.
 */
void cvmx_pki_show_valid_pcam_entries(int node);

/**
 * This function shows the pkind attributes in readable format,
 * read directly from hardware.
 * @param node  Node number.
 * @param pkind  PKIND number to print.
 */
void cvmx_pki_show_pkind_attributes(int node, int pkind);

/**
 * @INTERNAL
 * This function is called by cvmx_helper_shutdown() to extract all FPA buffers
 * out of the PKI. After this function completes, all FPA buffers that were
 * prefetched by PKI will be in the appropriate FPA pool.
 * This functions does not reset the PKI.
 * WARNING: It is very important that PKI be reset soon after a call to this function.
 *
 * @param node  Node number.
 */
void __cvmx_pki_free_ptr(int node);

#endif
