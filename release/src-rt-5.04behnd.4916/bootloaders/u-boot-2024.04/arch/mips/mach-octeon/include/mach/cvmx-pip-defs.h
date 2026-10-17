/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon pip.
 */

#ifndef __CVMX_PIP_DEFS_H__
#define __CVMX_PIP_DEFS_H__

#define CVMX_PIP_ALT_SKIP_CFGX(offset)	     (0x00011800A0002A00ull + ((offset) & 3) * 8)
#define CVMX_PIP_BCK_PRS		     (0x00011800A0000038ull)
#define CVMX_PIP_BIST_STATUS		     (0x00011800A0000000ull)
#define CVMX_PIP_BSEL_EXT_CFGX(offset)	     (0x00011800A0002800ull + ((offset) & 3) * 16)
#define CVMX_PIP_BSEL_EXT_POSX(offset)	     (0x00011800A0002808ull + ((offset) & 3) * 16)
#define CVMX_PIP_BSEL_TBL_ENTX(offset)	     (0x00011800A0003000ull + ((offset) & 511) * 8)
#define CVMX_PIP_CLKEN			     (0x00011800A0000040ull)
#define CVMX_PIP_CRC_CTLX(offset)	     (0x00011800A0000040ull + ((offset) & 1) * 8)
#define CVMX_PIP_CRC_IVX(offset)	     (0x00011800A0000050ull + ((offset) & 1) * 8)
#define CVMX_PIP_DEC_IPSECX(offset)	     (0x00011800A0000080ull + ((offset) & 3) * 8)
#define CVMX_PIP_DSA_SRC_GRP		     (0x00011800A0000190ull)
#define CVMX_PIP_DSA_VID_GRP		     (0x00011800A0000198ull)
#define CVMX_PIP_FRM_LEN_CHKX(offset)	     (0x00011800A0000180ull + ((offset) & 1) * 8)
#define CVMX_PIP_GBL_CFG		     (0x00011800A0000028ull)
#define CVMX_PIP_GBL_CTL		     (0x00011800A0000020ull)
#define CVMX_PIP_HG_PRI_QOS		     (0x00011800A00001A0ull)
#define CVMX_PIP_INT_EN			     (0x00011800A0000010ull)
#define CVMX_PIP_INT_REG		     (0x00011800A0000008ull)
#define CVMX_PIP_IP_OFFSET		     (0x00011800A0000060ull)
#define CVMX_PIP_PRI_TBLX(offset)	     (0x00011800A0004000ull + ((offset) & 255) * 8)
#define CVMX_PIP_PRT_CFGBX(offset)	     (0x00011800A0008000ull + ((offset) & 63) * 8)
#define CVMX_PIP_PRT_CFGX(offset)	     (0x00011800A0000200ull + ((offset) & 63) * 8)
#define CVMX_PIP_PRT_TAGX(offset)	     (0x00011800A0000400ull + ((offset) & 63) * 8)
#define CVMX_PIP_QOS_DIFFX(offset)	     (0x00011800A0000600ull + ((offset) & 63) * 8)
#define CVMX_PIP_QOS_VLANX(offset)	     (0x00011800A00000C0ull + ((offset) & 7) * 8)
#define CVMX_PIP_QOS_WATCHX(offset)	     (0x00011800A0000100ull + ((offset) & 7) * 8)
#define CVMX_PIP_RAW_WORD		     (0x00011800A00000B0ull)
#define CVMX_PIP_SFT_RST		     (0x00011800A0000030ull)
#define CVMX_PIP_STAT0_PRTX(offset)	     (0x00011800A0000800ull + ((offset) & 63) * 80)
#define CVMX_PIP_STAT0_X(offset)	     (0x00011800A0040000ull + ((offset) & 63) * 128)
#define CVMX_PIP_STAT10_PRTX(offset)	     (0x00011800A0001480ull + ((offset) & 63) * 16)
#define CVMX_PIP_STAT10_X(offset)	     (0x00011800A0040050ull + ((offset) & 63) * 128)
#define CVMX_PIP_STAT11_PRTX(offset)	     (0x00011800A0001488ull + ((offset) & 63) * 16)
#define CVMX_PIP_STAT11_X(offset)	     (0x00011800A0040058ull + ((offset) & 63) * 128)
#define CVMX_PIP_STAT1_PRTX(offset)	     (0x00011800A0000808ull + ((offset) & 63) * 80)
#define CVMX_PIP_STAT1_X(offset)	     (0x00011800A0040008ull + ((offset) & 63) * 128)
#define CVMX_PIP_STAT2_PRTX(offset)	     (0x00011800A0000810ull + ((offset) & 63) * 80)
#define CVMX_PIP_STAT2_X(offset)	     (0x00011800A0040010ull + ((offset) & 63) * 128)
#define CVMX_PIP_STAT3_PRTX(offset)	     (0x00011800A0000818ull + ((offset) & 63) * 80)
#define CVMX_PIP_STAT3_X(offset)	     (0x00011800A0040018ull + ((offset) & 63) * 128)
#define CVMX_PIP_STAT4_PRTX(offset)	     (0x00011800A0000820ull + ((offset) & 63) * 80)
#define CVMX_PIP_STAT4_X(offset)	     (0x00011800A0040020ull + ((offset) & 63) * 128)
#define CVMX_PIP_STAT5_PRTX(offset)	     (0x00011800A0000828ull + ((offset) & 63) * 80)
#define CVMX_PIP_STAT5_X(offset)	     (0x00011800A0040028ull + ((offset) & 63) * 128)
#define CVMX_PIP_STAT6_PRTX(offset)	     (0x00011800A0000830ull + ((offset) & 63) * 80)
#define CVMX_PIP_STAT6_X(offset)	     (0x00011800A0040030ull + ((offset) & 63) * 128)
#define CVMX_PIP_STAT7_PRTX(offset)	     (0x00011800A0000838ull + ((offset) & 63) * 80)
#define CVMX_PIP_STAT7_X(offset)	     (0x00011800A0040038ull + ((offset) & 63) * 128)
#define CVMX_PIP_STAT8_PRTX(offset)	     (0x00011800A0000840ull + ((offset) & 63) * 80)
#define CVMX_PIP_STAT8_X(offset)	     (0x00011800A0040040ull + ((offset) & 63) * 128)
#define CVMX_PIP_STAT9_PRTX(offset)	     (0x00011800A0000848ull + ((offset) & 63) * 80)
#define CVMX_PIP_STAT9_X(offset)	     (0x00011800A0040048ull + ((offset) & 63) * 128)
#define CVMX_PIP_STAT_CTL		     (0x00011800A0000018ull)
#define CVMX_PIP_STAT_INB_ERRSX(offset)	     (0x00011800A0001A10ull + ((offset) & 63) * 32)
#define CVMX_PIP_STAT_INB_ERRS_PKNDX(offset) (0x00011800A0020010ull + ((offset) & 63) * 32)
#define CVMX_PIP_STAT_INB_OCTSX(offset)	     (0x00011800A0001A08ull + ((offset) & 63) * 32)
#define CVMX_PIP_STAT_INB_OCTS_PKNDX(offset) (0x00011800A0020008ull + ((offset) & 63) * 32)
#define CVMX_PIP_STAT_INB_PKTSX(offset)	     (0x00011800A0001A00ull + ((offset) & 63) * 32)
#define CVMX_PIP_STAT_INB_PKTS_PKNDX(offset) (0x00011800A0020000ull + ((offset) & 63) * 32)
#define CVMX_PIP_SUB_PKIND_FCSX(offset)	     (0x00011800A0080000ull)
#define CVMX_PIP_TAG_INCX(offset)	     (0x00011800A0001800ull + ((offset) & 63) * 8)
#define CVMX_PIP_TAG_MASK		     (0x00011800A0000070ull)
#define CVMX_PIP_TAG_SECRET		     (0x00011800A0000068ull)
#define CVMX_PIP_TODO_ENTRY		     (0x00011800A0000078ull)
#define CVMX_PIP_VLAN_ETYPESX(offset)	     (0x00011800A00001C0ull + ((offset) & 1) * 8)
#define CVMX_PIP_XSTAT0_PRTX(offset)	     (0x00011800A0002000ull + ((offset) & 63) * 80 - 80 * 40)
#define CVMX_PIP_XSTAT10_PRTX(offset)	     (0x00011800A0001700ull + ((offset) & 63) * 16 - 16 * 40)
#define CVMX_PIP_XSTAT11_PRTX(offset)	     (0x00011800A0001708ull + ((offset) & 63) * 16 - 16 * 40)
#define CVMX_PIP_XSTAT1_PRTX(offset)	     (0x00011800A0002008ull + ((offset) & 63) * 80 - 80 * 40)
#define CVMX_PIP_XSTAT2_PRTX(offset)	     (0x00011800A0002010ull + ((offset) & 63) * 80 - 80 * 40)
#define CVMX_PIP_XSTAT3_PRTX(offset)	     (0x00011800A0002018ull + ((offset) & 63) * 80 - 80 * 40)
#define CVMX_PIP_XSTAT4_PRTX(offset)	     (0x00011800A0002020ull + ((offset) & 63) * 80 - 80 * 40)
#define CVMX_PIP_XSTAT5_PRTX(offset)	     (0x00011800A0002028ull + ((offset) & 63) * 80 - 80 * 40)
#define CVMX_PIP_XSTAT6_PRTX(offset)	     (0x00011800A0002030ull + ((offset) & 63) * 80 - 80 * 40)
#define CVMX_PIP_XSTAT7_PRTX(offset)	     (0x00011800A0002038ull + ((offset) & 63) * 80 - 80 * 40)
#define CVMX_PIP_XSTAT8_PRTX(offset)	     (0x00011800A0002040ull + ((offset) & 63) * 80 - 80 * 40)
#define CVMX_PIP_XSTAT9_PRTX(offset)	     (0x00011800A0002048ull + ((offset) & 63) * 80 - 80 * 40)

/**
 * cvmx_pip_alt_skip_cfg#
 *
 * Notes:
 * The actual SKIP I determined by HW is based on the packet contents.  BIT0 and
 * BIT1 make up a two value value that the selects the skip value as follows.
 *
 *    lookup_value = LEN ? ( packet_in_bits[BIT1], packet_in_bits[BIT0] ) : ( 0, packet_in_bits[BIT0] );
 *    SKIP I       = lookup_value == 3 ? SKIP3 :
 *                   lookup_value == 2 ? SKIP2 :
 *                   lookup_value == 1 ? SKIP1 :
 *                   PIP_PRT_CFG<pknd>[SKIP];
 */
union cvmx_pip_alt_skip_cfgx {
	u64 u64;
	struct cvmx_pip_alt_skip_cfgx_s {
		u64 reserved_57_63 : 7;
		u64 len : 1;
		u64 reserved_46_55 : 10;
		u64 bit1 : 6;
		u64 reserved_38_39 : 2;
		u64 bit0 : 6;
		u64 reserved_23_31 : 9;
		u64 skip3 : 7;
		u64 reserved_15_15 : 1;
		u64 skip2 : 7;
		u64 reserved_7_7 : 1;
		u64 skip1 : 7;
	} s;
	struct cvmx_pip_alt_skip_cfgx_s cn61xx;
	struct cvmx_pip_alt_skip_cfgx_s cn66xx;
	struct cvmx_pip_alt_skip_cfgx_s cn68xx;
	struct cvmx_pip_alt_skip_cfgx_s cn70xx;
	struct cvmx_pip_alt_skip_cfgx_s cn70xxp1;
	struct cvmx_pip_alt_skip_cfgx_s cnf71xx;
};

typedef union cvmx_pip_alt_skip_cfgx cvmx_pip_alt_skip_cfgx_t;

/**
 * cvmx_pip_bck_prs
 *
 * When to assert backpressure based on the todo list filling up
 *
 */
union cvmx_pip_bck_prs {
	u64 u64;
	struct cvmx_pip_bck_prs_s {
		u64 bckprs : 1;
		u64 reserved_13_62 : 50;
		u64 hiwater : 5;
		u64 reserved_5_7 : 3;
		u64 lowater : 5;
	} s;
	struct cvmx_pip_bck_prs_s cn38xx;
	struct cvmx_pip_bck_prs_s cn38xxp2;
	struct cvmx_pip_bck_prs_s cn56xx;
	struct cvmx_pip_bck_prs_s cn56xxp1;
	struct cvmx_pip_bck_prs_s cn58xx;
	struct cvmx_pip_bck_prs_s cn58xxp1;
	struct cvmx_pip_bck_prs_s cn61xx;
	struct cvmx_pip_bck_prs_s cn63xx;
	struct cvmx_pip_bck_prs_s cn63xxp1;
	struct cvmx_pip_bck_prs_s cn66xx;
	struct cvmx_pip_bck_prs_s cn68xx;
	struct cvmx_pip_bck_prs_s cn68xxp1;
	struct cvmx_pip_bck_prs_s cn70xx;
	struct cvmx_pip_bck_prs_s cn70xxp1;
	struct cvmx_pip_bck_prs_s cnf71xx;
};

typedef union cvmx_pip_bck_prs cvmx_pip_bck_prs_t;

/**
 * cvmx_pip_bist_status
 *
 * PIP_BIST_STATUS = PIP's BIST Results
 *
 */
union cvmx_pip_bist_status {
	u64 u64;
	struct cvmx_pip_bist_status_s {
		u64 reserved_22_63 : 42;
		u64 bist : 22;
	} s;
	struct cvmx_pip_bist_status_cn30xx {
		u64 reserved_18_63 : 46;
		u64 bist : 18;
	} cn30xx;
	struct cvmx_pip_bist_status_cn30xx cn31xx;
	struct cvmx_pip_bist_status_cn30xx cn38xx;
	struct cvmx_pip_bist_status_cn30xx cn38xxp2;
	struct cvmx_pip_bist_status_cn50xx {
		u64 reserved_17_63 : 47;
		u64 bist : 17;
	} cn50xx;
	struct cvmx_pip_bist_status_cn30xx cn52xx;
	struct cvmx_pip_bist_status_cn30xx cn52xxp1;
	struct cvmx_pip_bist_status_cn30xx cn56xx;
	struct cvmx_pip_bist_status_cn30xx cn56xxp1;
	struct cvmx_pip_bist_status_cn30xx cn58xx;
	struct cvmx_pip_bist_status_cn30xx cn58xxp1;
	struct cvmx_pip_bist_status_cn61xx {
		u64 reserved_20_63 : 44;
		u64 bist : 20;
	} cn61xx;
	struct cvmx_pip_bist_status_cn30xx cn63xx;
	struct cvmx_pip_bist_status_cn30xx cn63xxp1;
	struct cvmx_pip_bist_status_cn61xx cn66xx;
	struct cvmx_pip_bist_status_s cn68xx;
	struct cvmx_pip_bist_status_cn61xx cn68xxp1;
	struct cvmx_pip_bist_status_cn61xx cn70xx;
	struct cvmx_pip_bist_status_cn61xx cn70xxp1;
	struct cvmx_pip_bist_status_cn61xx cnf71xx;
};

typedef union cvmx_pip_bist_status cvmx_pip_bist_status_t;

/**
 * cvmx_pip_bsel_ext_cfg#
 *
 * tag, offset, and skip values to be used when using the corresponding extractor.
 *
 */
union cvmx_pip_bsel_ext_cfgx {
	u64 u64;
	struct cvmx_pip_bsel_ext_cfgx_s {
		u64 reserved_56_63 : 8;
		u64 upper_tag : 16;
		u64 tag : 8;
		u64 reserved_25_31 : 7;
		u64 offset : 9;
		u64 reserved_7_15 : 9;
		u64 skip : 7;
	} s;
	struct cvmx_pip_bsel_ext_cfgx_s cn61xx;
	struct cvmx_pip_bsel_ext_cfgx_s cn68xx;
	struct cvmx_pip_bsel_ext_cfgx_s cn70xx;
	struct cvmx_pip_bsel_ext_cfgx_s cn70xxp1;
	struct cvmx_pip_bsel_ext_cfgx_s cnf71xx;
};

typedef union cvmx_pip_bsel_ext_cfgx cvmx_pip_bsel_ext_cfgx_t;

/**
 * cvmx_pip_bsel_ext_pos#
 *
 * bit positions and valids to be used when using the corresponding extractor.
 *
 */
union cvmx_pip_bsel_ext_posx {
	u64 u64;
	struct cvmx_pip_bsel_ext_posx_s {
		u64 pos7_val : 1;
		u64 pos7 : 7;
		u64 pos6_val : 1;
		u64 pos6 : 7;
		u64 pos5_val : 1;
		u64 pos5 : 7;
		u64 pos4_val : 1;
		u64 pos4 : 7;
		u64 pos3_val : 1;
		u64 pos3 : 7;
		u64 pos2_val : 1;
		u64 pos2 : 7;
		u64 pos1_val : 1;
		u64 pos1 : 7;
		u64 pos0_val : 1;
		u64 pos0 : 7;
	} s;
	struct cvmx_pip_bsel_ext_posx_s cn61xx;
	struct cvmx_pip_bsel_ext_posx_s cn68xx;
	struct cvmx_pip_bsel_ext_posx_s cn70xx;
	struct cvmx_pip_bsel_ext_posx_s cn70xxp1;
	struct cvmx_pip_bsel_ext_posx_s cnf71xx;
};

typedef union cvmx_pip_bsel_ext_posx cvmx_pip_bsel_ext_posx_t;

/**
 * cvmx_pip_bsel_tbl_ent#
 *
 * PIP_BSEL_TBL_ENTX = Entry for the extractor table
 *
 */
union cvmx_pip_bsel_tbl_entx {
	u64 u64;
	struct cvmx_pip_bsel_tbl_entx_s {
		u64 tag_en : 1;
		u64 grp_en : 1;
		u64 tt_en : 1;
		u64 qos_en : 1;
		u64 reserved_40_59 : 20;
		u64 tag : 8;
		u64 reserved_22_31 : 10;
		u64 grp : 6;
		u64 reserved_10_15 : 6;
		u64 tt : 2;
		u64 reserved_3_7 : 5;
		u64 qos : 3;
	} s;
	struct cvmx_pip_bsel_tbl_entx_cn61xx {
		u64 tag_en : 1;
		u64 grp_en : 1;
		u64 tt_en : 1;
		u64 qos_en : 1;
		u64 reserved_40_59 : 20;
		u64 tag : 8;
		u64 reserved_20_31 : 12;
		u64 grp : 4;
		u64 reserved_10_15 : 6;
		u64 tt : 2;
		u64 reserved_3_7 : 5;
		u64 qos : 3;
	} cn61xx;
	struct cvmx_pip_bsel_tbl_entx_s cn68xx;
	struct cvmx_pip_bsel_tbl_entx_cn61xx cn70xx;
	struct cvmx_pip_bsel_tbl_entx_cn61xx cn70xxp1;
	struct cvmx_pip_bsel_tbl_entx_cn61xx cnf71xx;
};

typedef union cvmx_pip_bsel_tbl_entx cvmx_pip_bsel_tbl_entx_t;

/**
 * cvmx_pip_clken
 */
union cvmx_pip_clken {
	u64 u64;
	struct cvmx_pip_clken_s {
		u64 reserved_1_63 : 63;
		u64 clken : 1;
	} s;
	struct cvmx_pip_clken_s cn61xx;
	struct cvmx_pip_clken_s cn63xx;
	struct cvmx_pip_clken_s cn63xxp1;
	struct cvmx_pip_clken_s cn66xx;
	struct cvmx_pip_clken_s cn68xx;
	struct cvmx_pip_clken_s cn68xxp1;
	struct cvmx_pip_clken_s cn70xx;
	struct cvmx_pip_clken_s cn70xxp1;
	struct cvmx_pip_clken_s cnf71xx;
};

typedef union cvmx_pip_clken cvmx_pip_clken_t;

/**
 * cvmx_pip_crc_ctl#
 *
 * PIP_CRC_CTL = PIP CRC Control Register
 *
 * Controls datapath reflection when calculating CRC
 */
union cvmx_pip_crc_ctlx {
	u64 u64;
	struct cvmx_pip_crc_ctlx_s {
		u64 reserved_2_63 : 62;
		u64 invres : 1;
		u64 reflect : 1;
	} s;
	struct cvmx_pip_crc_ctlx_s cn38xx;
	struct cvmx_pip_crc_ctlx_s cn38xxp2;
	struct cvmx_pip_crc_ctlx_s cn58xx;
	struct cvmx_pip_crc_ctlx_s cn58xxp1;
};

typedef union cvmx_pip_crc_ctlx cvmx_pip_crc_ctlx_t;

/**
 * cvmx_pip_crc_iv#
 *
 * PIP_CRC_IV = PIP CRC IV Register
 *
 * Determines the IV used by the CRC algorithm
 *
 * Notes:
 * * PIP_CRC_IV
 * PIP_CRC_IV controls the initial state of the CRC algorithm.  Octane can
 * support a wide range of CRC algorithms and as such, the IV must be
 * carefully constructed to meet the specific algorithm.  The code below
 * determines the value to program into Octane based on the algorthim's IV
 * and width.  In the case of Octane, the width should always be 32.
 *
 * PIP_CRC_IV0 sets the IV for ports 0-15 while PIP_CRC_IV1 sets the IV for
 * ports 16-31.
 *
 *  unsigned octane_crc_iv(unsigned algorithm_iv, unsigned poly, unsigned w)
 *  [
 *    int i;
 *    int doit;
 *    unsigned int current_val = algorithm_iv;
 *
 *    for(i = 0; i < w; i++) [
 *      doit = current_val & 0x1;
 *
 *      if(doit) current_val ^= poly;
 *      assert(!(current_val & 0x1));
 *
 *      current_val = (current_val >> 1) | (doit << (w-1));
 *    ]
 *
 *    return current_val;
 *  ]
 */
union cvmx_pip_crc_ivx {
	u64 u64;
	struct cvmx_pip_crc_ivx_s {
		u64 reserved_32_63 : 32;
		u64 iv : 32;
	} s;
	struct cvmx_pip_crc_ivx_s cn38xx;
	struct cvmx_pip_crc_ivx_s cn38xxp2;
	struct cvmx_pip_crc_ivx_s cn58xx;
	struct cvmx_pip_crc_ivx_s cn58xxp1;
};

typedef union cvmx_pip_crc_ivx cvmx_pip_crc_ivx_t;

/**
 * cvmx_pip_dec_ipsec#
 *
 * PIP sets the dec_ipsec based on TCP or UDP destination port.
 *
 */
union cvmx_pip_dec_ipsecx {
	u64 u64;
	struct cvmx_pip_dec_ipsecx_s {
		u64 reserved_18_63 : 46;
		u64 tcp : 1;
		u64 udp : 1;
		u64 dprt : 16;
	} s;
	struct cvmx_pip_dec_ipsecx_s cn30xx;
	struct cvmx_pip_dec_ipsecx_s cn31xx;
	struct cvmx_pip_dec_ipsecx_s cn38xx;
	struct cvmx_pip_dec_ipsecx_s cn38xxp2;
	struct cvmx_pip_dec_ipsecx_s cn50xx;
	struct cvmx_pip_dec_ipsecx_s cn52xx;
	struct cvmx_pip_dec_ipsecx_s cn52xxp1;
	struct cvmx_pip_dec_ipsecx_s cn56xx;
	struct cvmx_pip_dec_ipsecx_s cn56xxp1;
	struct cvmx_pip_dec_ipsecx_s cn58xx;
	struct cvmx_pip_dec_ipsecx_s cn58xxp1;
	struct cvmx_pip_dec_ipsecx_s cn61xx;
	struct cvmx_pip_dec_ipsecx_s cn63xx;
	struct cvmx_pip_dec_ipsecx_s cn63xxp1;
	struct cvmx_pip_dec_ipsecx_s cn66xx;
	struct cvmx_pip_dec_ipsecx_s cn68xx;
	struct cvmx_pip_dec_ipsecx_s cn68xxp1;
	struct cvmx_pip_dec_ipsecx_s cn70xx;
	struct cvmx_pip_dec_ipsecx_s cn70xxp1;
	struct cvmx_pip_dec_ipsecx_s cnf71xx;
};

typedef union cvmx_pip_dec_ipsecx cvmx_pip_dec_ipsecx_t;

/**
 * cvmx_pip_dsa_src_grp
 */
union cvmx_pip_dsa_src_grp {
	u64 u64;
	struct cvmx_pip_dsa_src_grp_s {
		u64 map15 : 4;
		u64 map14 : 4;
		u64 map13 : 4;
		u64 map12 : 4;
		u64 map11 : 4;
		u64 map10 : 4;
		u64 map9 : 4;
		u64 map8 : 4;
		u64 map7 : 4;
		u64 map6 : 4;
		u64 map5 : 4;
		u64 map4 : 4;
		u64 map3 : 4;
		u64 map2 : 4;
		u64 map1 : 4;
		u64 map0 : 4;
	} s;
	struct cvmx_pip_dsa_src_grp_s cn52xx;
	struct cvmx_pip_dsa_src_grp_s cn52xxp1;
	struct cvmx_pip_dsa_src_grp_s cn56xx;
	struct cvmx_pip_dsa_src_grp_s cn61xx;
	struct cvmx_pip_dsa_src_grp_s cn63xx;
	struct cvmx_pip_dsa_src_grp_s cn63xxp1;
	struct cvmx_pip_dsa_src_grp_s cn66xx;
	struct cvmx_pip_dsa_src_grp_s cn68xx;
	struct cvmx_pip_dsa_src_grp_s cn68xxp1;
	struct cvmx_pip_dsa_src_grp_s cn70xx;
	struct cvmx_pip_dsa_src_grp_s cn70xxp1;
	struct cvmx_pip_dsa_src_grp_s cnf71xx;
};

typedef union cvmx_pip_dsa_src_grp cvmx_pip_dsa_src_grp_t;

/**
 * cvmx_pip_dsa_vid_grp
 */
union cvmx_pip_dsa_vid_grp {
	u64 u64;
	struct cvmx_pip_dsa_vid_grp_s {
		u64 map15 : 4;
		u64 map14 : 4;
		u64 map13 : 4;
		u64 map12 : 4;
		u64 map11 : 4;
		u64 map10 : 4;
		u64 map9 : 4;
		u64 map8 : 4;
		u64 map7 : 4;
		u64 map6 : 4;
		u64 map5 : 4;
		u64 map4 : 4;
		u64 map3 : 4;
		u64 map2 : 4;
		u64 map1 : 4;
		u64 map0 : 4;
	} s;
	struct cvmx_pip_dsa_vid_grp_s cn52xx;
	struct cvmx_pip_dsa_vid_grp_s cn52xxp1;
	struct cvmx_pip_dsa_vid_grp_s cn56xx;
	struct cvmx_pip_dsa_vid_grp_s cn61xx;
	struct cvmx_pip_dsa_vid_grp_s cn63xx;
	struct cvmx_pip_dsa_vid_grp_s cn63xxp1;
	struct cvmx_pip_dsa_vid_grp_s cn66xx;
	struct cvmx_pip_dsa_vid_grp_s cn68xx;
	struct cvmx_pip_dsa_vid_grp_s cn68xxp1;
	struct cvmx_pip_dsa_vid_grp_s cn70xx;
	struct cvmx_pip_dsa_vid_grp_s cn70xxp1;
	struct cvmx_pip_dsa_vid_grp_s cnf71xx;
};

typedef union cvmx_pip_dsa_vid_grp cvmx_pip_dsa_vid_grp_t;

/**
 * cvmx_pip_frm_len_chk#
 *
 * Notes:
 * PIP_FRM_LEN_CHK0 is used for packets on packet interface0, PCI, PCI RAW, and PKO loopback ports.
 * PIP_FRM_LEN_CHK1 is unused.
 */
union cvmx_pip_frm_len_chkx {
	u64 u64;
	struct cvmx_pip_frm_len_chkx_s {
		u64 reserved_32_63 : 32;
		u64 maxlen : 16;
		u64 minlen : 16;
	} s;
	struct cvmx_pip_frm_len_chkx_s cn50xx;
	struct cvmx_pip_frm_len_chkx_s cn52xx;
	struct cvmx_pip_frm_len_chkx_s cn52xxp1;
	struct cvmx_pip_frm_len_chkx_s cn56xx;
	struct cvmx_pip_frm_len_chkx_s cn56xxp1;
	struct cvmx_pip_frm_len_chkx_s cn61xx;
	struct cvmx_pip_frm_len_chkx_s cn63xx;
	struct cvmx_pip_frm_len_chkx_s cn63xxp1;
	struct cvmx_pip_frm_len_chkx_s cn66xx;
	struct cvmx_pip_frm_len_chkx_s cn68xx;
	struct cvmx_pip_frm_len_chkx_s cn68xxp1;
	struct cvmx_pip_frm_len_chkx_s cn70xx;
	struct cvmx_pip_frm_len_chkx_s cn70xxp1;
	struct cvmx_pip_frm_len_chkx_s cnf71xx;
};

typedef union cvmx_pip_frm_len_chkx cvmx_pip_frm_len_chkx_t;

/**
 * cvmx_pip_gbl_cfg
 *
 * Global config information that applies to all ports.
 *
 */
union cvmx_pip_gbl_cfg {
	u64 u64;
	struct cvmx_pip_gbl_cfg_s {
		u64 reserved_19_63 : 45;
		u64 tag_syn : 1;
		u64 ip6_udp : 1;
		u64 max_l2 : 1;
		u64 reserved_11_15 : 5;
		u64 raw_shf : 3;
		u64 reserved_3_7 : 5;
		u64 nip_shf : 3;
	} s;
	struct cvmx_pip_gbl_cfg_s cn30xx;
	struct cvmx_pip_gbl_cfg_s cn31xx;
	struct cvmx_pip_gbl_cfg_s cn38xx;
	struct cvmx_pip_gbl_cfg_s cn38xxp2;
	struct cvmx_pip_gbl_cfg_s cn50xx;
	struct cvmx_pip_gbl_cfg_s cn52xx;
	struct cvmx_pip_gbl_cfg_s cn52xxp1;
	struct cvmx_pip_gbl_cfg_s cn56xx;
	struct cvmx_pip_gbl_cfg_s cn56xxp1;
	struct cvmx_pip_gbl_cfg_s cn58xx;
	struct cvmx_pip_gbl_cfg_s cn58xxp1;
	struct cvmx_pip_gbl_cfg_s cn61xx;
	struct cvmx_pip_gbl_cfg_s cn63xx;
	struct cvmx_pip_gbl_cfg_s cn63xxp1;
	struct cvmx_pip_gbl_cfg_s cn66xx;
	struct cvmx_pip_gbl_cfg_s cn68xx;
	struct cvmx_pip_gbl_cfg_s cn68xxp1;
	struct cvmx_pip_gbl_cfg_s cn70xx;
	struct cvmx_pip_gbl_cfg_s cn70xxp1;
	struct cvmx_pip_gbl_cfg_s cnf71xx;
};

typedef union cvmx_pip_gbl_cfg cvmx_pip_gbl_cfg_t;

/**
 * cvmx_pip_gbl_ctl
 *
 * Global control information.  These are the global checker enables for
 * IPv4/IPv6 and TCP/UDP parsing.  The enables effect all ports.
 */
union cvmx_pip_gbl_ctl {
	u64 u64;
	struct cvmx_pip_gbl_ctl_s {
		u64 reserved_29_63 : 35;
		u64 egrp_dis : 1;
		u64 ihmsk_dis : 1;
		u64 dsa_grp_tvid : 1;
		u64 dsa_grp_scmd : 1;
		u64 dsa_grp_sid : 1;
		u64 reserved_21_23 : 3;
		u64 ring_en : 1;
		u64 reserved_17_19 : 3;
		u64 ignrs : 1;
		u64 vs_wqe : 1;
		u64 vs_qos : 1;
		u64 l2_mal : 1;
		u64 tcp_flag : 1;
		u64 l4_len : 1;
		u64 l4_chk : 1;
		u64 l4_prt : 1;
		u64 l4_mal : 1;
		u64 reserved_6_7 : 2;
		u64 ip6_eext : 2;
		u64 ip4_opts : 1;
		u64 ip_hop : 1;
		u64 ip_mal : 1;
		u64 ip_chk : 1;
	} s;
	struct cvmx_pip_gbl_ctl_cn30xx {
		u64 reserved_17_63 : 47;
		u64 ignrs : 1;
		u64 vs_wqe : 1;
		u64 vs_qos : 1;
		u64 l2_mal : 1;
		u64 tcp_flag : 1;
		u64 l4_len : 1;
		u64 l4_chk : 1;
		u64 l4_prt : 1;
		u64 l4_mal : 1;
		u64 reserved_6_7 : 2;
		u64 ip6_eext : 2;
		u64 ip4_opts : 1;
		u64 ip_hop : 1;
		u64 ip_mal : 1;
		u64 ip_chk : 1;
	} cn30xx;
	struct cvmx_pip_gbl_ctl_cn30xx cn31xx;
	struct cvmx_pip_gbl_ctl_cn30xx cn38xx;
	struct cvmx_pip_gbl_ctl_cn30xx cn38xxp2;
	struct cvmx_pip_gbl_ctl_cn30xx cn50xx;
	struct cvmx_pip_gbl_ctl_cn52xx {
		u64 reserved_27_63 : 37;
		u64 dsa_grp_tvid : 1;
		u64 dsa_grp_scmd : 1;
		u64 dsa_grp_sid : 1;
		u64 reserved_21_23 : 3;
		u64 ring_en : 1;
		u64 reserved_17_19 : 3;
		u64 ignrs : 1;
		u64 vs_wqe : 1;
		u64 vs_qos : 1;
		u64 l2_mal : 1;
		u64 tcp_flag : 1;
		u64 l4_len : 1;
		u64 l4_chk : 1;
		u64 l4_prt : 1;
		u64 l4_mal : 1;
		u64 reserved_6_7 : 2;
		u64 ip6_eext : 2;
		u64 ip4_opts : 1;
		u64 ip_hop : 1;
		u64 ip_mal : 1;
		u64 ip_chk : 1;
	} cn52xx;
	struct cvmx_pip_gbl_ctl_cn52xx cn52xxp1;
	struct cvmx_pip_gbl_ctl_cn52xx cn56xx;
	struct cvmx_pip_gbl_ctl_cn56xxp1 {
		u64 reserved_21_63 : 43;
		u64 ring_en : 1;
		u64 reserved_17_19 : 3;
		u64 ignrs : 1;
		u64 vs_wqe : 1;
		u64 vs_qos : 1;
		u64 l2_mal : 1;
		u64 tcp_flag : 1;
		u64 l4_len : 1;
		u64 l4_chk : 1;
		u64 l4_prt : 1;
		u64 l4_mal : 1;
		u64 reserved_6_7 : 2;
		u64 ip6_eext : 2;
		u64 ip4_opts : 1;
		u64 ip_hop : 1;
		u64 ip_mal : 1;
		u64 ip_chk : 1;
	} cn56xxp1;
	struct cvmx_pip_gbl_ctl_cn30xx cn58xx;
	struct cvmx_pip_gbl_ctl_cn30xx cn58xxp1;
	struct cvmx_pip_gbl_ctl_cn61xx {
		u64 reserved_28_63 : 36;
		u64 ihmsk_dis : 1;
		u64 dsa_grp_tvid : 1;
		u64 dsa_grp_scmd : 1;
		u64 dsa_grp_sid : 1;
		u64 reserved_21_23 : 3;
		u64 ring_en : 1;
		u64 reserved_17_19 : 3;
		u64 ignrs : 1;
		u64 vs_wqe : 1;
		u64 vs_qos : 1;
		u64 l2_mal : 1;
		u64 tcp_flag : 1;
		u64 l4_len : 1;
		u64 l4_chk : 1;
		u64 l4_prt : 1;
		u64 l4_mal : 1;
		u64 reserved_6_7 : 2;
		u64 ip6_eext : 2;
		u64 ip4_opts : 1;
		u64 ip_hop : 1;
		u64 ip_mal : 1;
		u64 ip_chk : 1;
	} cn61xx;
	struct cvmx_pip_gbl_ctl_cn61xx cn63xx;
	struct cvmx_pip_gbl_ctl_cn61xx cn63xxp1;
	struct cvmx_pip_gbl_ctl_cn61xx cn66xx;
	struct cvmx_pip_gbl_ctl_cn68xx {
		u64 reserved_29_63 : 35;
		u64 egrp_dis : 1;
		u64 ihmsk_dis : 1;
		u64 dsa_grp_tvid : 1;
		u64 dsa_grp_scmd : 1;
		u64 dsa_grp_sid : 1;
		u64 reserved_17_23 : 7;
		u64 ignrs : 1;
		u64 vs_wqe : 1;
		u64 vs_qos : 1;
		u64 l2_mal : 1;
		u64 tcp_flag : 1;
		u64 l4_len : 1;
		u64 l4_chk : 1;
		u64 l4_prt : 1;
		u64 l4_mal : 1;
		u64 reserved_6_7 : 2;
		u64 ip6_eext : 2;
		u64 ip4_opts : 1;
		u64 ip_hop : 1;
		u64 ip_mal : 1;
		u64 ip_chk : 1;
	} cn68xx;
	struct cvmx_pip_gbl_ctl_cn68xxp1 {
		u64 reserved_28_63 : 36;
		u64 ihmsk_dis : 1;
		u64 dsa_grp_tvid : 1;
		u64 dsa_grp_scmd : 1;
		u64 dsa_grp_sid : 1;
		u64 reserved_17_23 : 7;
		u64 ignrs : 1;
		u64 vs_wqe : 1;
		u64 vs_qos : 1;
		u64 l2_mal : 1;
		u64 tcp_flag : 1;
		u64 l4_len : 1;
		u64 l4_chk : 1;
		u64 l4_prt : 1;
		u64 l4_mal : 1;
		u64 reserved_6_7 : 2;
		u64 ip6_eext : 2;
		u64 ip4_opts : 1;
		u64 ip_hop : 1;
		u64 ip_mal : 1;
		u64 ip_chk : 1;
	} cn68xxp1;
	struct cvmx_pip_gbl_ctl_cn61xx cn70xx;
	struct cvmx_pip_gbl_ctl_cn61xx cn70xxp1;
	struct cvmx_pip_gbl_ctl_cn61xx cnf71xx;
};

typedef union cvmx_pip_gbl_ctl cvmx_pip_gbl_ctl_t;

/**
 * cvmx_pip_hg_pri_qos
 *
 * Notes:
 * This register controls accesses to the HG_QOS_TABLE.  To write an entry of
 * the table, write PIP_HG_PRI_QOS with PRI=table address, QOS=priority level,
 * UP_QOS=1.  To read an entry of the table, write PIP_HG_PRI_QOS with
 * PRI=table address, QOS=dont_carepriority level, UP_QOS=0 and then read
 * PIP_HG_PRI_QOS.  The table data will be in PIP_HG_PRI_QOS[QOS].
 */
union cvmx_pip_hg_pri_qos {
	u64 u64;
	struct cvmx_pip_hg_pri_qos_s {
		u64 reserved_13_63 : 51;
		u64 up_qos : 1;
		u64 reserved_11_11 : 1;
		u64 qos : 3;
		u64 reserved_6_7 : 2;
		u64 pri : 6;
	} s;
	struct cvmx_pip_hg_pri_qos_s cn52xx;
	struct cvmx_pip_hg_pri_qos_s cn52xxp1;
	struct cvmx_pip_hg_pri_qos_s cn56xx;
	struct cvmx_pip_hg_pri_qos_s cn61xx;
	struct cvmx_pip_hg_pri_qos_s cn63xx;
	struct cvmx_pip_hg_pri_qos_s cn63xxp1;
	struct cvmx_pip_hg_pri_qos_s cn66xx;
	struct cvmx_pip_hg_pri_qos_s cn70xx;
	struct cvmx_pip_hg_pri_qos_s cn70xxp1;
	struct cvmx_pip_hg_pri_qos_s cnf71xx;
};

typedef union cvmx_pip_hg_pri_qos cvmx_pip_hg_pri_qos_t;

/**
 * cvmx_pip_int_en
 *
 * Determines if hardward should raise an interrupt to software
 * when an exception event occurs.
 */
union cvmx_pip_int_en {
	u64 u64;
	struct cvmx_pip_int_en_s {
		u64 reserved_13_63 : 51;
		u64 punyerr : 1;
		u64 lenerr : 1;
		u64 maxerr : 1;
		u64 minerr : 1;
		u64 beperr : 1;
		u64 feperr : 1;
		u64 todoovr : 1;
		u64 skprunt : 1;
		u64 badtag : 1;
		u64 prtnxa : 1;
		u64 bckprs : 1;
		u64 crcerr : 1;
		u64 pktdrp : 1;
	} s;
	struct cvmx_pip_int_en_cn30xx {
		u64 reserved_9_63 : 55;
		u64 beperr : 1;
		u64 feperr : 1;
		u64 todoovr : 1;
		u64 skprunt : 1;
		u64 badtag : 1;
		u64 prtnxa : 1;
		u64 bckprs : 1;
		u64 crcerr : 1;
		u64 pktdrp : 1;
	} cn30xx;
	struct cvmx_pip_int_en_cn30xx cn31xx;
	struct cvmx_pip_int_en_cn30xx cn38xx;
	struct cvmx_pip_int_en_cn30xx cn38xxp2;
	struct cvmx_pip_int_en_cn50xx {
		u64 reserved_12_63 : 52;
		u64 lenerr : 1;
		u64 maxerr : 1;
		u64 minerr : 1;
		u64 beperr : 1;
		u64 feperr : 1;
		u64 todoovr : 1;
		u64 skprunt : 1;
		u64 badtag : 1;
		u64 prtnxa : 1;
		u64 bckprs : 1;
		u64 reserved_1_1 : 1;
		u64 pktdrp : 1;
	} cn50xx;
	struct cvmx_pip_int_en_cn52xx {
		u64 reserved_13_63 : 51;
		u64 punyerr : 1;
		u64 lenerr : 1;
		u64 maxerr : 1;
		u64 minerr : 1;
		u64 beperr : 1;
		u64 feperr : 1;
		u64 todoovr : 1;
		u64 skprunt : 1;
		u64 badtag : 1;
		u64 prtnxa : 1;
		u64 bckprs : 1;
		u64 reserved_1_1 : 1;
		u64 pktdrp : 1;
	} cn52xx;
	struct cvmx_pip_int_en_cn52xx cn52xxp1;
	struct cvmx_pip_int_en_s cn56xx;
	struct cvmx_pip_int_en_cn56xxp1 {
		u64 reserved_12_63 : 52;
		u64 lenerr : 1;
		u64 maxerr : 1;
		u64 minerr : 1;
		u64 beperr : 1;
		u64 feperr : 1;
		u64 todoovr : 1;
		u64 skprunt : 1;
		u64 badtag : 1;
		u64 prtnxa : 1;
		u64 bckprs : 1;
		u64 crcerr : 1;
		u64 pktdrp : 1;
	} cn56xxp1;
	struct cvmx_pip_int_en_cn58xx {
		u64 reserved_13_63 : 51;
		u64 punyerr : 1;
		u64 reserved_9_11 : 3;
		u64 beperr : 1;
		u64 feperr : 1;
		u64 todoovr : 1;
		u64 skprunt : 1;
		u64 badtag : 1;
		u64 prtnxa : 1;
		u64 bckprs : 1;
		u64 crcerr : 1;
		u64 pktdrp : 1;
	} cn58xx;
	struct cvmx_pip_int_en_cn30xx cn58xxp1;
	struct cvmx_pip_int_en_s cn61xx;
	struct cvmx_pip_int_en_s cn63xx;
	struct cvmx_pip_int_en_s cn63xxp1;
	struct cvmx_pip_int_en_s cn66xx;
	struct cvmx_pip_int_en_s cn68xx;
	struct cvmx_pip_int_en_s cn68xxp1;
	struct cvmx_pip_int_en_s cn70xx;
	struct cvmx_pip_int_en_s cn70xxp1;
	struct cvmx_pip_int_en_s cnf71xx;
};

typedef union cvmx_pip_int_en cvmx_pip_int_en_t;

/**
 * cvmx_pip_int_reg
 *
 * Any exception event that occurs is captured in the PIP_INT_REG.
 * PIP_INT_REG will set the exception bit regardless of the value
 * of PIP_INT_EN.  PIP_INT_EN only controls if an interrupt is
 * raised to software.
 */
union cvmx_pip_int_reg {
	u64 u64;
	struct cvmx_pip_int_reg_s {
		u64 reserved_13_63 : 51;
		u64 punyerr : 1;
		u64 lenerr : 1;
		u64 maxerr : 1;
		u64 minerr : 1;
		u64 beperr : 1;
		u64 feperr : 1;
		u64 todoovr : 1;
		u64 skprunt : 1;
		u64 badtag : 1;
		u64 prtnxa : 1;
		u64 bckprs : 1;
		u64 crcerr : 1;
		u64 pktdrp : 1;
	} s;
	struct cvmx_pip_int_reg_cn30xx {
		u64 reserved_9_63 : 55;
		u64 beperr : 1;
		u64 feperr : 1;
		u64 todoovr : 1;
		u64 skprunt : 1;
		u64 badtag : 1;
		u64 prtnxa : 1;
		u64 bckprs : 1;
		u64 crcerr : 1;
		u64 pktdrp : 1;
	} cn30xx;
	struct cvmx_pip_int_reg_cn30xx cn31xx;
	struct cvmx_pip_int_reg_cn30xx cn38xx;
	struct cvmx_pip_int_reg_cn30xx cn38xxp2;
	struct cvmx_pip_int_reg_cn50xx {
		u64 reserved_12_63 : 52;
		u64 lenerr : 1;
		u64 maxerr : 1;
		u64 minerr : 1;
		u64 beperr : 1;
		u64 feperr : 1;
		u64 todoovr : 1;
		u64 skprunt : 1;
		u64 badtag : 1;
		u64 prtnxa : 1;
		u64 bckprs : 1;
		u64 reserved_1_1 : 1;
		u64 pktdrp : 1;
	} cn50xx;
	struct cvmx_pip_int_reg_cn52xx {
		u64 reserved_13_63 : 51;
		u64 punyerr : 1;
		u64 lenerr : 1;
		u64 maxerr : 1;
		u64 minerr : 1;
		u64 beperr : 1;
		u64 feperr : 1;
		u64 todoovr : 1;
		u64 skprunt : 1;
		u64 badtag : 1;
		u64 prtnxa : 1;
		u64 bckprs : 1;
		u64 reserved_1_1 : 1;
		u64 pktdrp : 1;
	} cn52xx;
	struct cvmx_pip_int_reg_cn52xx cn52xxp1;
	struct cvmx_pip_int_reg_s cn56xx;
	struct cvmx_pip_int_reg_cn56xxp1 {
		u64 reserved_12_63 : 52;
		u64 lenerr : 1;
		u64 maxerr : 1;
		u64 minerr : 1;
		u64 beperr : 1;
		u64 feperr : 1;
		u64 todoovr : 1;
		u64 skprunt : 1;
		u64 badtag : 1;
		u64 prtnxa : 1;
		u64 bckprs : 1;
		u64 crcerr : 1;
		u64 pktdrp : 1;
	} cn56xxp1;
	struct cvmx_pip_int_reg_cn58xx {
		u64 reserved_13_63 : 51;
		u64 punyerr : 1;
		u64 reserved_9_11 : 3;
		u64 beperr : 1;
		u64 feperr : 1;
		u64 todoovr : 1;
		u64 skprunt : 1;
		u64 badtag : 1;
		u64 prtnxa : 1;
		u64 bckprs : 1;
		u64 crcerr : 1;
		u64 pktdrp : 1;
	} cn58xx;
	struct cvmx_pip_int_reg_cn30xx cn58xxp1;
	struct cvmx_pip_int_reg_s cn61xx;
	struct cvmx_pip_int_reg_s cn63xx;
	struct cvmx_pip_int_reg_s cn63xxp1;
	struct cvmx_pip_int_reg_s cn66xx;
	struct cvmx_pip_int_reg_s cn68xx;
	struct cvmx_pip_int_reg_s cn68xxp1;
	struct cvmx_pip_int_reg_s cn70xx;
	struct cvmx_pip_int_reg_s cn70xxp1;
	struct cvmx_pip_int_reg_s cnf71xx;
};

typedef union cvmx_pip_int_reg cvmx_pip_int_reg_t;

/**
 * cvmx_pip_ip_offset
 *
 * An 8-byte offset to find the start of the IP header in the data portion of IP workQ entires
 *
 */
union cvmx_pip_ip_offset {
	u64 u64;
	struct cvmx_pip_ip_offset_s {
		u64 reserved_3_63 : 61;
		u64 offset : 3;
	} s;
	struct cvmx_pip_ip_offset_s cn30xx;
	struct cvmx_pip_ip_offset_s cn31xx;
	struct cvmx_pip_ip_offset_s cn38xx;
	struct cvmx_pip_ip_offset_s cn38xxp2;
	struct cvmx_pip_ip_offset_s cn50xx;
	struct cvmx_pip_ip_offset_s cn52xx;
	struct cvmx_pip_ip_offset_s cn52xxp1;
	struct cvmx_pip_ip_offset_s cn56xx;
	struct cvmx_pip_ip_offset_s cn56xxp1;
	struct cvmx_pip_ip_offset_s cn58xx;
	struct cvmx_pip_ip_offset_s cn58xxp1;
	struct cvmx_pip_ip_offset_s cn61xx;
	struct cvmx_pip_ip_offset_s cn63xx;
	struct cvmx_pip_ip_offset_s cn63xxp1;
	struct cvmx_pip_ip_offset_s cn66xx;
	struct cvmx_pip_ip_offset_s cn68xx;
	struct cvmx_pip_ip_offset_s cn68xxp1;
	struct cvmx_pip_ip_offset_s cn70xx;
	struct cvmx_pip_ip_offset_s cn70xxp1;
	struct cvmx_pip_ip_offset_s cnf71xx;
};

typedef union cvmx_pip_ip_offset cvmx_pip_ip_offset_t;

/**
 * cvmx_pip_pri_tbl#
 *
 * Notes:
 * The priority level from HiGig header is as follows
 *
 * HiGig/HiGig+ PRI = [1'b0, CNG[1:0], COS[2:0]]
 * HiGig2       PRI = [DP[1:0], TC[3:0]]
 *
 * DSA          PRI = WORD0[15:13]
 *
 * VLAN         PRI = VLAN[15:13]
 *
 * DIFFSERV         = IP.TOS/CLASS<7:2>
 */
union cvmx_pip_pri_tblx {
	u64 u64;
	struct cvmx_pip_pri_tblx_s {
		u64 diff2_padd : 8;
		u64 hg2_padd : 8;
		u64 vlan2_padd : 8;
		u64 reserved_38_39 : 2;
		u64 diff2_bpid : 6;
		u64 reserved_30_31 : 2;
		u64 hg2_bpid : 6;
		u64 reserved_22_23 : 2;
		u64 vlan2_bpid : 6;
		u64 reserved_11_15 : 5;
		u64 diff2_qos : 3;
		u64 reserved_7_7 : 1;
		u64 hg2_qos : 3;
		u64 reserved_3_3 : 1;
		u64 vlan2_qos : 3;
	} s;
	struct cvmx_pip_pri_tblx_s cn68xx;
	struct cvmx_pip_pri_tblx_s cn68xxp1;
};

typedef union cvmx_pip_pri_tblx cvmx_pip_pri_tblx_t;

/**
 * cvmx_pip_prt_cfg#
 *
 * PIP_PRT_CFGX = Per port config information
 *
 */
union cvmx_pip_prt_cfgx {
	u64 u64;
	struct cvmx_pip_prt_cfgx_s {
		u64 reserved_55_63 : 9;
		u64 ih_pri : 1;
		u64 len_chk_sel : 1;
		u64 pad_len : 1;
		u64 vlan_len : 1;
		u64 lenerr_en : 1;
		u64 maxerr_en : 1;
		u64 minerr_en : 1;
		u64 grp_wat_47 : 4;
		u64 qos_wat_47 : 4;
		u64 reserved_37_39 : 3;
		u64 rawdrp : 1;
		u64 tag_inc : 2;
		u64 dyn_rs : 1;
		u64 inst_hdr : 1;
		u64 grp_wat : 4;
		u64 hg_qos : 1;
		u64 qos : 3;
		u64 qos_wat : 4;
		u64 qos_vsel : 1;
		u64 qos_vod : 1;
		u64 qos_diff : 1;
		u64 qos_vlan : 1;
		u64 reserved_13_15 : 3;
		u64 crc_en : 1;
		u64 higig_en : 1;
		u64 dsa_en : 1;
		cvmx_pip_port_parse_mode_t mode : 2;
		u64 reserved_7_7 : 1;
		u64 skip : 7;
	} s;
	struct cvmx_pip_prt_cfgx_cn30xx {
		u64 reserved_37_63 : 27;
		u64 rawdrp : 1;
		u64 tag_inc : 2;
		u64 dyn_rs : 1;
		u64 inst_hdr : 1;
		u64 grp_wat : 4;
		u64 reserved_27_27 : 1;
		u64 qos : 3;
		u64 qos_wat : 4;
		u64 reserved_18_19 : 2;
		u64 qos_diff : 1;
		u64 qos_vlan : 1;
		u64 reserved_10_15 : 6;
		cvmx_pip_port_parse_mode_t mode : 2;
		u64 reserved_7_7 : 1;
		u64 skip : 7;
	} cn30xx;
	struct cvmx_pip_prt_cfgx_cn30xx cn31xx;
	struct cvmx_pip_prt_cfgx_cn38xx {
		u64 reserved_37_63 : 27;
		u64 rawdrp : 1;
		u64 tag_inc : 2;
		u64 dyn_rs : 1;
		u64 inst_hdr : 1;
		u64 grp_wat : 4;
		u64 reserved_27_27 : 1;
		u64 qos : 3;
		u64 qos_wat : 4;
		u64 reserved_18_19 : 2;
		u64 qos_diff : 1;
		u64 qos_vlan : 1;
		u64 reserved_13_15 : 3;
		u64 crc_en : 1;
		u64 reserved_10_11 : 2;
		cvmx_pip_port_parse_mode_t mode : 2;
		u64 reserved_7_7 : 1;
		u64 skip : 7;
	} cn38xx;
	struct cvmx_pip_prt_cfgx_cn38xx cn38xxp2;
	struct cvmx_pip_prt_cfgx_cn50xx {
		u64 reserved_53_63 : 11;
		u64 pad_len : 1;
		u64 vlan_len : 1;
		u64 lenerr_en : 1;
		u64 maxerr_en : 1;
		u64 minerr_en : 1;
		u64 grp_wat_47 : 4;
		u64 qos_wat_47 : 4;
		u64 reserved_37_39 : 3;
		u64 rawdrp : 1;
		u64 tag_inc : 2;
		u64 dyn_rs : 1;
		u64 inst_hdr : 1;
		u64 grp_wat : 4;
		u64 reserved_27_27 : 1;
		u64 qos : 3;
		u64 qos_wat : 4;
		u64 reserved_19_19 : 1;
		u64 qos_vod : 1;
		u64 qos_diff : 1;
		u64 qos_vlan : 1;
		u64 reserved_13_15 : 3;
		u64 crc_en : 1;
		u64 reserved_10_11 : 2;
		cvmx_pip_port_parse_mode_t mode : 2;
		u64 reserved_7_7 : 1;
		u64 skip : 7;
	} cn50xx;
	struct cvmx_pip_prt_cfgx_cn52xx {
		u64 reserved_53_63 : 11;
		u64 pad_len : 1;
		u64 vlan_len : 1;
		u64 lenerr_en : 1;
		u64 maxerr_en : 1;
		u64 minerr_en : 1;
		u64 grp_wat_47 : 4;
		u64 qos_wat_47 : 4;
		u64 reserved_37_39 : 3;
		u64 rawdrp : 1;
		u64 tag_inc : 2;
		u64 dyn_rs : 1;
		u64 inst_hdr : 1;
		u64 grp_wat : 4;
		u64 hg_qos : 1;
		u64 qos : 3;
		u64 qos_wat : 4;
		u64 qos_vsel : 1;
		u64 qos_vod : 1;
		u64 qos_diff : 1;
		u64 qos_vlan : 1;
		u64 reserved_13_15 : 3;
		u64 crc_en : 1;
		u64 higig_en : 1;
		u64 dsa_en : 1;
		cvmx_pip_port_parse_mode_t mode : 2;
		u64 reserved_7_7 : 1;
		u64 skip : 7;
	} cn52xx;
	struct cvmx_pip_prt_cfgx_cn52xx cn52xxp1;
	struct cvmx_pip_prt_cfgx_cn52xx cn56xx;
	struct cvmx_pip_prt_cfgx_cn50xx cn56xxp1;
	struct cvmx_pip_prt_cfgx_cn58xx {
		u64 reserved_37_63 : 27;
		u64 rawdrp : 1;
		u64 tag_inc : 2;
		u64 dyn_rs : 1;
		u64 inst_hdr : 1;
		u64 grp_wat : 4;
		u64 reserved_27_27 : 1;
		u64 qos : 3;
		u64 qos_wat : 4;
		u64 reserved_19_19 : 1;
		u64 qos_vod : 1;
		u64 qos_diff : 1;
		u64 qos_vlan : 1;
		u64 reserved_13_15 : 3;
		u64 crc_en : 1;
		u64 reserved_10_11 : 2;
		cvmx_pip_port_parse_mode_t mode : 2;
		u64 reserved_7_7 : 1;
		u64 skip : 7;
	} cn58xx;
	struct cvmx_pip_prt_cfgx_cn58xx cn58xxp1;
	struct cvmx_pip_prt_cfgx_cn52xx cn61xx;
	struct cvmx_pip_prt_cfgx_cn52xx cn63xx;
	struct cvmx_pip_prt_cfgx_cn52xx cn63xxp1;
	struct cvmx_pip_prt_cfgx_cn52xx cn66xx;
	struct cvmx_pip_prt_cfgx_cn68xx {
		u64 reserved_55_63 : 9;
		u64 ih_pri : 1;
		u64 len_chk_sel : 1;
		u64 pad_len : 1;
		u64 vlan_len : 1;
		u64 lenerr_en : 1;
		u64 maxerr_en : 1;
		u64 minerr_en : 1;
		u64 grp_wat_47 : 4;
		u64 qos_wat_47 : 4;
		u64 reserved_37_39 : 3;
		u64 rawdrp : 1;
		u64 tag_inc : 2;
		u64 dyn_rs : 1;
		u64 inst_hdr : 1;
		u64 grp_wat : 4;
		u64 hg_qos : 1;
		u64 qos : 3;
		u64 qos_wat : 4;
		u64 reserved_19_19 : 1;
		u64 qos_vod : 1;
		u64 qos_diff : 1;
		u64 qos_vlan : 1;
		u64 reserved_13_15 : 3;
		u64 crc_en : 1;
		u64 higig_en : 1;
		u64 dsa_en : 1;
		cvmx_pip_port_parse_mode_t mode : 2;
		u64 reserved_7_7 : 1;
		u64 skip : 7;
	} cn68xx;
	struct cvmx_pip_prt_cfgx_cn68xx cn68xxp1;
	struct cvmx_pip_prt_cfgx_cn52xx cn70xx;
	struct cvmx_pip_prt_cfgx_cn52xx cn70xxp1;
	struct cvmx_pip_prt_cfgx_cn52xx cnf71xx;
};

typedef union cvmx_pip_prt_cfgx cvmx_pip_prt_cfgx_t;

/**
 * cvmx_pip_prt_cfgb#
 *
 * Notes:
 * PIP_PRT_CFGB* does not exist prior to pass 1.2.
 *
 */
union cvmx_pip_prt_cfgbx {
	u64 u64;
	struct cvmx_pip_prt_cfgbx_s {
		u64 reserved_39_63 : 25;
		u64 alt_skp_sel : 2;
		u64 alt_skp_en : 1;
		u64 reserved_35_35 : 1;
		u64 bsel_num : 2;
		u64 bsel_en : 1;
		u64 reserved_24_31 : 8;
		u64 base : 8;
		u64 reserved_6_15 : 10;
		u64 bpid : 6;
	} s;
	struct cvmx_pip_prt_cfgbx_cn61xx {
		u64 reserved_39_63 : 25;
		u64 alt_skp_sel : 2;
		u64 alt_skp_en : 1;
		u64 reserved_35_35 : 1;
		u64 bsel_num : 2;
		u64 bsel_en : 1;
		u64 reserved_0_31 : 32;
	} cn61xx;
	struct cvmx_pip_prt_cfgbx_cn66xx {
		u64 reserved_39_63 : 25;
		u64 alt_skp_sel : 2;
		u64 alt_skp_en : 1;
		u64 reserved_0_35 : 36;
	} cn66xx;
	struct cvmx_pip_prt_cfgbx_s cn68xx;
	struct cvmx_pip_prt_cfgbx_cn68xxp1 {
		u64 reserved_24_63 : 40;
		u64 base : 8;
		u64 reserved_6_15 : 10;
		u64 bpid : 6;
	} cn68xxp1;
	struct cvmx_pip_prt_cfgbx_cn61xx cn70xx;
	struct cvmx_pip_prt_cfgbx_cn61xx cn70xxp1;
	struct cvmx_pip_prt_cfgbx_cn61xx cnf71xx;
};

typedef union cvmx_pip_prt_cfgbx cvmx_pip_prt_cfgbx_t;

/**
 * cvmx_pip_prt_tag#
 *
 * PIP_PRT_TAGX = Per port config information
 *
 */
union cvmx_pip_prt_tagx {
	u64 u64;
	struct cvmx_pip_prt_tagx_s {
		u64 reserved_54_63 : 10;
		u64 portadd_en : 1;
		u64 inc_hwchk : 1;
		u64 reserved_50_51 : 2;
		u64 grptagbase_msb : 2;
		u64 reserved_46_47 : 2;
		u64 grptagmask_msb : 2;
		u64 reserved_42_43 : 2;
		u64 grp_msb : 2;
		u64 grptagbase : 4;
		u64 grptagmask : 4;
		u64 grptag : 1;
		u64 grptag_mskip : 1;
		u64 tag_mode : 2;
		u64 inc_vs : 2;
		u64 inc_vlan : 1;
		u64 inc_prt_flag : 1;
		u64 ip6_dprt_flag : 1;
		u64 ip4_dprt_flag : 1;
		u64 ip6_sprt_flag : 1;
		u64 ip4_sprt_flag : 1;
		u64 ip6_nxth_flag : 1;
		u64 ip4_pctl_flag : 1;
		u64 ip6_dst_flag : 1;
		u64 ip4_dst_flag : 1;
		u64 ip6_src_flag : 1;
		u64 ip4_src_flag : 1;
		cvmx_pow_tag_type_t tcp6_tag_type : 2;
		cvmx_pow_tag_type_t tcp4_tag_type : 2;
		cvmx_pow_tag_type_t ip6_tag_type : 2;
		cvmx_pow_tag_type_t ip4_tag_type : 2;
		cvmx_pow_tag_type_t non_tag_type : 2;
		u64 grp : 4;
	} s;
	struct cvmx_pip_prt_tagx_cn30xx {
		u64 reserved_40_63 : 24;
		u64 grptagbase : 4;
		u64 grptagmask : 4;
		u64 grptag : 1;
		u64 reserved_30_30 : 1;
		u64 tag_mode : 2;
		u64 inc_vs : 2;
		u64 inc_vlan : 1;
		u64 inc_prt_flag : 1;
		u64 ip6_dprt_flag : 1;
		u64 ip4_dprt_flag : 1;
		u64 ip6_sprt_flag : 1;
		u64 ip4_sprt_flag : 1;
		u64 ip6_nxth_flag : 1;
		u64 ip4_pctl_flag : 1;
		u64 ip6_dst_flag : 1;
		u64 ip4_dst_flag : 1;
		u64 ip6_src_flag : 1;
		u64 ip4_src_flag : 1;
		cvmx_pow_tag_type_t tcp6_tag_type : 2;
		cvmx_pow_tag_type_t tcp4_tag_type : 2;
		cvmx_pow_tag_type_t ip6_tag_type : 2;
		cvmx_pow_tag_type_t ip4_tag_type : 2;
		cvmx_pow_tag_type_t non_tag_type : 2;
		u64 grp : 4;
	} cn30xx;
	struct cvmx_pip_prt_tagx_cn30xx cn31xx;
	struct cvmx_pip_prt_tagx_cn30xx cn38xx;
	struct cvmx_pip_prt_tagx_cn30xx cn38xxp2;
	struct cvmx_pip_prt_tagx_cn50xx {
		u64 reserved_40_63 : 24;
		u64 grptagbase : 4;
		u64 grptagmask : 4;
		u64 grptag : 1;
		u64 grptag_mskip : 1;
		u64 tag_mode : 2;
		u64 inc_vs : 2;
		u64 inc_vlan : 1;
		u64 inc_prt_flag : 1;
		u64 ip6_dprt_flag : 1;
		u64 ip4_dprt_flag : 1;
		u64 ip6_sprt_flag : 1;
		u64 ip4_sprt_flag : 1;
		u64 ip6_nxth_flag : 1;
		u64 ip4_pctl_flag : 1;
		u64 ip6_dst_flag : 1;
		u64 ip4_dst_flag : 1;
		u64 ip6_src_flag : 1;
		u64 ip4_src_flag : 1;
		cvmx_pow_tag_type_t tcp6_tag_type : 2;
		cvmx_pow_tag_type_t tcp4_tag_type : 2;
		cvmx_pow_tag_type_t ip6_tag_type : 2;
		cvmx_pow_tag_type_t ip4_tag_type : 2;
		cvmx_pow_tag_type_t non_tag_type : 2;
		u64 grp : 4;
	} cn50xx;
	struct cvmx_pip_prt_tagx_cn50xx cn52xx;
	struct cvmx_pip_prt_tagx_cn50xx cn52xxp1;
	struct cvmx_pip_prt_tagx_cn50xx cn56xx;
	struct cvmx_pip_prt_tagx_cn50xx cn56xxp1;
	struct cvmx_pip_prt_tagx_cn30xx cn58xx;
	struct cvmx_pip_prt_tagx_cn30xx cn58xxp1;
	struct cvmx_pip_prt_tagx_cn50xx cn61xx;
	struct cvmx_pip_prt_tagx_cn50xx cn63xx;
	struct cvmx_pip_prt_tagx_cn50xx cn63xxp1;
	struct cvmx_pip_prt_tagx_cn50xx cn66xx;
	struct cvmx_pip_prt_tagx_s cn68xx;
	struct cvmx_pip_prt_tagx_s cn68xxp1;
	struct cvmx_pip_prt_tagx_cn50xx cn70xx;
	struct cvmx_pip_prt_tagx_cn50xx cn70xxp1;
	struct cvmx_pip_prt_tagx_cn50xx cnf71xx;
};

typedef union cvmx_pip_prt_tagx cvmx_pip_prt_tagx_t;

/**
 * cvmx_pip_qos_diff#
 *
 * PIP_QOS_DIFFX = QOS Diffserv Tables
 *
 */
union cvmx_pip_qos_diffx {
	u64 u64;
	struct cvmx_pip_qos_diffx_s {
		u64 reserved_3_63 : 61;
		u64 qos : 3;
	} s;
	struct cvmx_pip_qos_diffx_s cn30xx;
	struct cvmx_pip_qos_diffx_s cn31xx;
	struct cvmx_pip_qos_diffx_s cn38xx;
	struct cvmx_pip_qos_diffx_s cn38xxp2;
	struct cvmx_pip_qos_diffx_s cn50xx;
	struct cvmx_pip_qos_diffx_s cn52xx;
	struct cvmx_pip_qos_diffx_s cn52xxp1;
	struct cvmx_pip_qos_diffx_s cn56xx;
	struct cvmx_pip_qos_diffx_s cn56xxp1;
	struct cvmx_pip_qos_diffx_s cn58xx;
	struct cvmx_pip_qos_diffx_s cn58xxp1;
	struct cvmx_pip_qos_diffx_s cn61xx;
	struct cvmx_pip_qos_diffx_s cn63xx;
	struct cvmx_pip_qos_diffx_s cn63xxp1;
	struct cvmx_pip_qos_diffx_s cn66xx;
	struct cvmx_pip_qos_diffx_s cn70xx;
	struct cvmx_pip_qos_diffx_s cn70xxp1;
	struct cvmx_pip_qos_diffx_s cnf71xx;
};

typedef union cvmx_pip_qos_diffx cvmx_pip_qos_diffx_t;

/**
 * cvmx_pip_qos_vlan#
 *
 * If the PIP indentifies a packet is DSA/VLAN tagged, then the QOS
 * can be set based on the DSA/VLAN user priority.  These eight register
 * comprise the QOS values for all DSA/VLAN user priority values.
 */
union cvmx_pip_qos_vlanx {
	u64 u64;
	struct cvmx_pip_qos_vlanx_s {
		u64 reserved_7_63 : 57;
		u64 qos1 : 3;
		u64 reserved_3_3 : 1;
		u64 qos : 3;
	} s;
	struct cvmx_pip_qos_vlanx_cn30xx {
		u64 reserved_3_63 : 61;
		u64 qos : 3;
	} cn30xx;
	struct cvmx_pip_qos_vlanx_cn30xx cn31xx;
	struct cvmx_pip_qos_vlanx_cn30xx cn38xx;
	struct cvmx_pip_qos_vlanx_cn30xx cn38xxp2;
	struct cvmx_pip_qos_vlanx_cn30xx cn50xx;
	struct cvmx_pip_qos_vlanx_s cn52xx;
	struct cvmx_pip_qos_vlanx_s cn52xxp1;
	struct cvmx_pip_qos_vlanx_s cn56xx;
	struct cvmx_pip_qos_vlanx_cn30xx cn56xxp1;
	struct cvmx_pip_qos_vlanx_cn30xx cn58xx;
	struct cvmx_pip_qos_vlanx_cn30xx cn58xxp1;
	struct cvmx_pip_qos_vlanx_s cn61xx;
	struct cvmx_pip_qos_vlanx_s cn63xx;
	struct cvmx_pip_qos_vlanx_s cn63xxp1;
	struct cvmx_pip_qos_vlanx_s cn66xx;
	struct cvmx_pip_qos_vlanx_s cn70xx;
	struct cvmx_pip_qos_vlanx_s cn70xxp1;
	struct cvmx_pip_qos_vlanx_s cnf71xx;
};

typedef union cvmx_pip_qos_vlanx cvmx_pip_qos_vlanx_t;

/**
 * cvmx_pip_qos_watch#
 *
 * Sets up the Configuration CSRs for the four QOS Watchers.
 * Each Watcher can be set to look for a specific protocol,
 * TCP/UDP destination port, or Ethertype to override the
 * default QOS value.
 */
union cvmx_pip_qos_watchx {
	u64 u64;
	struct cvmx_pip_qos_watchx_s {
		u64 reserved_48_63 : 16;
		u64 mask : 16;
		u64 reserved_30_31 : 2;
		u64 grp : 6;
		u64 reserved_23_23 : 1;
		u64 qos : 3;
		u64 reserved_16_19 : 4;
		u64 match_value : 16;
	} s;
	struct cvmx_pip_qos_watchx_cn30xx {
		u64 reserved_48_63 : 16;
		u64 mask : 16;
		u64 reserved_28_31 : 4;
		u64 grp : 4;
		u64 reserved_23_23 : 1;
		u64 qos : 3;
		u64 reserved_18_19 : 2;

		cvmx_pip_qos_watch_types match_type : 2;
		u64 match_value : 16;
	} cn30xx;
	struct cvmx_pip_qos_watchx_cn30xx cn31xx;
	struct cvmx_pip_qos_watchx_cn30xx cn38xx;
	struct cvmx_pip_qos_watchx_cn30xx cn38xxp2;
	struct cvmx_pip_qos_watchx_cn50xx {
		u64 reserved_48_63 : 16;
		u64 mask : 16;
		u64 reserved_28_31 : 4;
		u64 grp : 4;
		u64 reserved_23_23 : 1;
		u64 qos : 3;
		u64 reserved_19_19 : 1;

		cvmx_pip_qos_watch_types match_type : 3;
		u64 match_value : 16;
	} cn50xx;
	struct cvmx_pip_qos_watchx_cn50xx cn52xx;
	struct cvmx_pip_qos_watchx_cn50xx cn52xxp1;
	struct cvmx_pip_qos_watchx_cn50xx cn56xx;
	struct cvmx_pip_qos_watchx_cn50xx cn56xxp1;
	struct cvmx_pip_qos_watchx_cn30xx cn58xx;
	struct cvmx_pip_qos_watchx_cn30xx cn58xxp1;
	struct cvmx_pip_qos_watchx_cn50xx cn61xx;
	struct cvmx_pip_qos_watchx_cn50xx cn63xx;
	struct cvmx_pip_qos_watchx_cn50xx cn63xxp1;
	struct cvmx_pip_qos_watchx_cn50xx cn66xx;
	struct cvmx_pip_qos_watchx_cn68xx {
		u64 reserved_48_63 : 16;
		u64 mask : 16;
		u64 reserved_30_31 : 2;
		u64 grp : 6;
		u64 reserved_23_23 : 1;
		u64 qos : 3;
		u64 reserved_19_19 : 1;

		cvmx_pip_qos_watch_types match_type : 3;
		u64 match_value : 16;
	} cn68xx;
	struct cvmx_pip_qos_watchx_cn68xx cn68xxp1;
	struct cvmx_pip_qos_watchx_cn70xx {
		u64 reserved_48_63 : 16;
		u64 mask : 16;
		u64 reserved_28_31 : 4;
		u64 grp : 4;
		u64 reserved_23_23 : 1;
		u64 qos : 3;
		u64 reserved_19_19 : 1;
		u64 typ : 3;
		u64 match_value : 16;
	} cn70xx;
	struct cvmx_pip_qos_watchx_cn70xx cn70xxp1;
	struct cvmx_pip_qos_watchx_cn50xx cnf71xx;
};

typedef union cvmx_pip_qos_watchx cvmx_pip_qos_watchx_t;

/**
 * cvmx_pip_raw_word
 *
 * The RAW Word2 to be inserted into the workQ entry of RAWFULL packets.
 *
 */
union cvmx_pip_raw_word {
	u64 u64;
	struct cvmx_pip_raw_word_s {
		u64 reserved_56_63 : 8;
		u64 word : 56;
	} s;
	struct cvmx_pip_raw_word_s cn30xx;
	struct cvmx_pip_raw_word_s cn31xx;
	struct cvmx_pip_raw_word_s cn38xx;
	struct cvmx_pip_raw_word_s cn38xxp2;
	struct cvmx_pip_raw_word_s cn50xx;
	struct cvmx_pip_raw_word_s cn52xx;
	struct cvmx_pip_raw_word_s cn52xxp1;
	struct cvmx_pip_raw_word_s cn56xx;
	struct cvmx_pip_raw_word_s cn56xxp1;
	struct cvmx_pip_raw_word_s cn58xx;
	struct cvmx_pip_raw_word_s cn58xxp1;
	struct cvmx_pip_raw_word_s cn61xx;
	struct cvmx_pip_raw_word_s cn63xx;
	struct cvmx_pip_raw_word_s cn63xxp1;
	struct cvmx_pip_raw_word_s cn66xx;
	struct cvmx_pip_raw_word_s cn68xx;
	struct cvmx_pip_raw_word_s cn68xxp1;
	struct cvmx_pip_raw_word_s cn70xx;
	struct cvmx_pip_raw_word_s cn70xxp1;
	struct cvmx_pip_raw_word_s cnf71xx;
};

typedef union cvmx_pip_raw_word cvmx_pip_raw_word_t;

/**
 * cvmx_pip_sft_rst
 *
 * When written to a '1', resets the pip block
 *
 */
union cvmx_pip_sft_rst {
	u64 u64;
	struct cvmx_pip_sft_rst_s {
		u64 reserved_1_63 : 63;
		u64 rst : 1;
	} s;
	struct cvmx_pip_sft_rst_s cn30xx;
	struct cvmx_pip_sft_rst_s cn31xx;
	struct cvmx_pip_sft_rst_s cn38xx;
	struct cvmx_pip_sft_rst_s cn50xx;
	struct cvmx_pip_sft_rst_s cn52xx;
	struct cvmx_pip_sft_rst_s cn52xxp1;
	struct cvmx_pip_sft_rst_s cn56xx;
	struct cvmx_pip_sft_rst_s cn56xxp1;
	struct cvmx_pip_sft_rst_s cn58xx;
	struct cvmx_pip_sft_rst_s cn58xxp1;
	struct cvmx_pip_sft_rst_s cn61xx;
	struct cvmx_pip_sft_rst_s cn63xx;
	struct cvmx_pip_sft_rst_s cn63xxp1;
	struct cvmx_pip_sft_rst_s cn66xx;
	struct cvmx_pip_sft_rst_s cn68xx;
	struct cvmx_pip_sft_rst_s cn68xxp1;
	struct cvmx_pip_sft_rst_s cn70xx;
	struct cvmx_pip_sft_rst_s cn70xxp1;
	struct cvmx_pip_sft_rst_s cnf71xx;
};

typedef union cvmx_pip_sft_rst cvmx_pip_sft_rst_t;

/**
 * cvmx_pip_stat0_#
 *
 * PIP Statistics Counters
 *
 * Note: special stat counter behavior
 *
 * 1) Read and write operations must arbitrate for the statistics resources
 *     along with the packet engines which are incrementing the counters.
 *     In order to not drop packet information, the packet HW is always a
 *     higher priority and the CSR requests will only be satisified when
 *     there are idle cycles.  This can potentially cause long delays if the
 *     system becomes full.
 *
 * 2) stat counters can be cleared in two ways.  If PIP_STAT_CTL[RDCLR] is
 *     set, then all read accesses will clear the register.  In addition,
 *     any write to a stats register will also reset the register to zero.
 *     Please note that the clearing operations must obey rule \#1 above.
 *
 * 3) all counters are wrapping - software must ensure they are read periodically
 *
 * 4) The counters accumulate statistics for packets that are sent to PKI.  If
 *    PTP_MODE is enabled, the 8B timestamp is prepended to the packet.  This
 *    additional 8B of data is captured in the octet counts.
 *
 * 5) X represents either the packet's port-kind or backpressure ID as
 *    determined by PIP_STAT_CTL[MODE]
 * PIP_STAT0_X = PIP_STAT_DRP_PKTS / PIP_STAT_DRP_OCTS
 */
union cvmx_pip_stat0_x {
	u64 u64;
	struct cvmx_pip_stat0_x_s {
		u64 drp_pkts : 32;
		u64 drp_octs : 32;
	} s;
	struct cvmx_pip_stat0_x_s cn68xx;
	struct cvmx_pip_stat0_x_s cn68xxp1;
};

typedef union cvmx_pip_stat0_x cvmx_pip_stat0_x_t;

/**
 * cvmx_pip_stat0_prt#
 *
 * "PIP Statistics Counters
 * Note: special stat counter behavior
 * 1) Read and write operations must arbitrate for the statistics resources
 * along with the packet engines which are incrementing the counters.
 * In order to not drop packet information, the packet HW is always a
 * higher priority and the CSR requests will only be satisified when
 * there are idle cycles.  This can potentially cause long delays if the
 * system becomes full.
 * 2) stat counters can be cleared in two ways.  If PIP_STAT_CTL[RDCLR] is
 * set, then all read accesses will clear the register.  In addition,
 * any write to a stats register will also reset the register to zero.
 * Please note that the clearing operations must obey rule \#1 above.
 * 3) all counters are wrapping - software must ensure they are read periodically
 * 4) The counters accumulate statistics for packets that are sent to PKI.  If
 * PTP_MODE is enabled, the 8B timestamp is prepended to the packet.  This
 * additional 8B of data is captured in the octet counts.
 * PIP_STAT0_PRT = PIP_STAT_DRP_PKTS / PIP_STAT_DRP_OCTS"
 */
union cvmx_pip_stat0_prtx {
	u64 u64;
	struct cvmx_pip_stat0_prtx_s {
		u64 drp_pkts : 32;
		u64 drp_octs : 32;
	} s;
	struct cvmx_pip_stat0_prtx_s cn30xx;
	struct cvmx_pip_stat0_prtx_s cn31xx;
	struct cvmx_pip_stat0_prtx_s cn38xx;
	struct cvmx_pip_stat0_prtx_s cn38xxp2;
	struct cvmx_pip_stat0_prtx_s cn50xx;
	struct cvmx_pip_stat0_prtx_s cn52xx;
	struct cvmx_pip_stat0_prtx_s cn52xxp1;
	struct cvmx_pip_stat0_prtx_s cn56xx;
	struct cvmx_pip_stat0_prtx_s cn56xxp1;
	struct cvmx_pip_stat0_prtx_s cn58xx;
	struct cvmx_pip_stat0_prtx_s cn58xxp1;
	struct cvmx_pip_stat0_prtx_s cn61xx;
	struct cvmx_pip_stat0_prtx_s cn63xx;
	struct cvmx_pip_stat0_prtx_s cn63xxp1;
	struct cvmx_pip_stat0_prtx_s cn66xx;
	struct cvmx_pip_stat0_prtx_s cn70xx;
	struct cvmx_pip_stat0_prtx_s cn70xxp1;
	struct cvmx_pip_stat0_prtx_s cnf71xx;
};

typedef union cvmx_pip_stat0_prtx cvmx_pip_stat0_prtx_t;

/**
 * cvmx_pip_stat10_#
 *
 * PIP_STAT10_X = PIP_STAT_L2_MCAST / PIP_STAT_L2_BCAST
 *
 */
union cvmx_pip_stat10_x {
	u64 u64;
	struct cvmx_pip_stat10_x_s {
		u64 bcast : 32;
		u64 mcast : 32;
	} s;
	struct cvmx_pip_stat10_x_s cn68xx;
	struct cvmx_pip_stat10_x_s cn68xxp1;
};

typedef union cvmx_pip_stat10_x cvmx_pip_stat10_x_t;

/**
 * cvmx_pip_stat10_prt#
 *
 * PIP_STAT10_PRTX = PIP_STAT_L2_MCAST / PIP_STAT_L2_BCAST
 *
 */
union cvmx_pip_stat10_prtx {
	u64 u64;
	struct cvmx_pip_stat10_prtx_s {
		u64 bcast : 32;
		u64 mcast : 32;
	} s;
	struct cvmx_pip_stat10_prtx_s cn52xx;
	struct cvmx_pip_stat10_prtx_s cn52xxp1;
	struct cvmx_pip_stat10_prtx_s cn56xx;
	struct cvmx_pip_stat10_prtx_s cn56xxp1;
	struct cvmx_pip_stat10_prtx_s cn61xx;
	struct cvmx_pip_stat10_prtx_s cn63xx;
	struct cvmx_pip_stat10_prtx_s cn63xxp1;
	struct cvmx_pip_stat10_prtx_s cn66xx;
	struct cvmx_pip_stat10_prtx_s cn70xx;
	struct cvmx_pip_stat10_prtx_s cn70xxp1;
	struct cvmx_pip_stat10_prtx_s cnf71xx;
};

typedef union cvmx_pip_stat10_prtx cvmx_pip_stat10_prtx_t;

/**
 * cvmx_pip_stat11_#
 *
 * PIP_STAT11_X = PIP_STAT_L3_MCAST / PIP_STAT_L3_BCAST
 *
 */
union cvmx_pip_stat11_x {
	u64 u64;
	struct cvmx_pip_stat11_x_s {
		u64 bcast : 32;
		u64 mcast : 32;
	} s;
	struct cvmx_pip_stat11_x_s cn68xx;
	struct cvmx_pip_stat11_x_s cn68xxp1;
};

typedef union cvmx_pip_stat11_x cvmx_pip_stat11_x_t;

/**
 * cvmx_pip_stat11_prt#
 *
 * PIP_STAT11_PRTX = PIP_STAT_L3_MCAST / PIP_STAT_L3_BCAST
 *
 */
union cvmx_pip_stat11_prtx {
	u64 u64;
	struct cvmx_pip_stat11_prtx_s {
		u64 bcast : 32;
		u64 mcast : 32;
	} s;
	struct cvmx_pip_stat11_prtx_s cn52xx;
	struct cvmx_pip_stat11_prtx_s cn52xxp1;
	struct cvmx_pip_stat11_prtx_s cn56xx;
	struct cvmx_pip_stat11_prtx_s cn56xxp1;
	struct cvmx_pip_stat11_prtx_s cn61xx;
	struct cvmx_pip_stat11_prtx_s cn63xx;
	struct cvmx_pip_stat11_prtx_s cn63xxp1;
	struct cvmx_pip_stat11_prtx_s cn66xx;
	struct cvmx_pip_stat11_prtx_s cn70xx;
	struct cvmx_pip_stat11_prtx_s cn70xxp1;
	struct cvmx_pip_stat11_prtx_s cnf71xx;
};

typedef union cvmx_pip_stat11_prtx cvmx_pip_stat11_prtx_t;

/**
 * cvmx_pip_stat1_#
 *
 * PIP_STAT1_X = PIP_STAT_OCTS
 *
 */
union cvmx_pip_stat1_x {
	u64 u64;
	struct cvmx_pip_stat1_x_s {
		u64 reserved_48_63 : 16;
		u64 octs : 48;
	} s;
	struct cvmx_pip_stat1_x_s cn68xx;
	struct cvmx_pip_stat1_x_s cn68xxp1;
};

typedef union cvmx_pip_stat1_x cvmx_pip_stat1_x_t;

/**
 * cvmx_pip_stat1_prt#
 *
 * PIP_STAT1_PRTX = PIP_STAT_OCTS
 *
 */
union cvmx_pip_stat1_prtx {
	u64 u64;
	struct cvmx_pip_stat1_prtx_s {
		u64 reserved_48_63 : 16;
		u64 octs : 48;
	} s;
	struct cvmx_pip_stat1_prtx_s cn30xx;
	struct cvmx_pip_stat1_prtx_s cn31xx;
	struct cvmx_pip_stat1_prtx_s cn38xx;
	struct cvmx_pip_stat1_prtx_s cn38xxp2;
	struct cvmx_pip_stat1_prtx_s cn50xx;
	struct cvmx_pip_stat1_prtx_s cn52xx;
	struct cvmx_pip_stat1_prtx_s cn52xxp1;
	struct cvmx_pip_stat1_prtx_s cn56xx;
	struct cvmx_pip_stat1_prtx_s cn56xxp1;
	struct cvmx_pip_stat1_prtx_s cn58xx;
	struct cvmx_pip_stat1_prtx_s cn58xxp1;
	struct cvmx_pip_stat1_prtx_s cn61xx;
	struct cvmx_pip_stat1_prtx_s cn63xx;
	struct cvmx_pip_stat1_prtx_s cn63xxp1;
	struct cvmx_pip_stat1_prtx_s cn66xx;
	struct cvmx_pip_stat1_prtx_s cn70xx;
	struct cvmx_pip_stat1_prtx_s cn70xxp1;
	struct cvmx_pip_stat1_prtx_s cnf71xx;
};

typedef union cvmx_pip_stat1_prtx cvmx_pip_stat1_prtx_t;

/**
 * cvmx_pip_stat2_#
 *
 * PIP_STAT2_X = PIP_STAT_PKTS     / PIP_STAT_RAW
 *
 */
union cvmx_pip_stat2_x {
	u64 u64;
	struct cvmx_pip_stat2_x_s {
		u64 pkts : 32;
		u64 raw : 32;
	} s;
	struct cvmx_pip_stat2_x_s cn68xx;
	struct cvmx_pip_stat2_x_s cn68xxp1;
};

typedef union cvmx_pip_stat2_x cvmx_pip_stat2_x_t;

/**
 * cvmx_pip_stat2_prt#
 *
 * PIP_STAT2_PRTX = PIP_STAT_PKTS     / PIP_STAT_RAW
 *
 */
union cvmx_pip_stat2_prtx {
	u64 u64;
	struct cvmx_pip_stat2_prtx_s {
		u64 pkts : 32;
		u64 raw : 32;
	} s;
	struct cvmx_pip_stat2_prtx_s cn30xx;
	struct cvmx_pip_stat2_prtx_s cn31xx;
	struct cvmx_pip_stat2_prtx_s cn38xx;
	struct cvmx_pip_stat2_prtx_s cn38xxp2;
	struct cvmx_pip_stat2_prtx_s cn50xx;
	struct cvmx_pip_stat2_prtx_s cn52xx;
	struct cvmx_pip_stat2_prtx_s cn52xxp1;
	struct cvmx_pip_stat2_prtx_s cn56xx;
	struct cvmx_pip_stat2_prtx_s cn56xxp1;
	struct cvmx_pip_stat2_prtx_s cn58xx;
	struct cvmx_pip_stat2_prtx_s cn58xxp1;
	struct cvmx_pip_stat2_prtx_s cn61xx;
	struct cvmx_pip_stat2_prtx_s cn63xx;
	struct cvmx_pip_stat2_prtx_s cn63xxp1;
	struct cvmx_pip_stat2_prtx_s cn66xx;
	struct cvmx_pip_stat2_prtx_s cn70xx;
	struct cvmx_pip_stat2_prtx_s cn70xxp1;
	struct cvmx_pip_stat2_prtx_s cnf71xx;
};

typedef union cvmx_pip_stat2_prtx cvmx_pip_stat2_prtx_t;

/**
 * cvmx_pip_stat3_#
 *
 * PIP_STAT3_X = PIP_STAT_BCST     / PIP_STAT_MCST
 *
 */
union cvmx_pip_stat3_x {
	u64 u64;
	struct cvmx_pip_stat3_x_s {
		u64 bcst : 32;
		u64 mcst : 32;
	} s;
	struct cvmx_pip_stat3_x_s cn68xx;
	struct cvmx_pip_stat3_x_s cn68xxp1;
};

typedef union cvmx_pip_stat3_x cvmx_pip_stat3_x_t;

/**
 * cvmx_pip_stat3_prt#
 *
 * PIP_STAT3_PRTX = PIP_STAT_BCST     / PIP_STAT_MCST
 *
 */
union cvmx_pip_stat3_prtx {
	u64 u64;
	struct cvmx_pip_stat3_prtx_s {
		u64 bcst : 32;
		u64 mcst : 32;
	} s;
	struct cvmx_pip_stat3_prtx_s cn30xx;
	struct cvmx_pip_stat3_prtx_s cn31xx;
	struct cvmx_pip_stat3_prtx_s cn38xx;
	struct cvmx_pip_stat3_prtx_s cn38xxp2;
	struct cvmx_pip_stat3_prtx_s cn50xx;
	struct cvmx_pip_stat3_prtx_s cn52xx;
	struct cvmx_pip_stat3_prtx_s cn52xxp1;
	struct cvmx_pip_stat3_prtx_s cn56xx;
	struct cvmx_pip_stat3_prtx_s cn56xxp1;
	struct cvmx_pip_stat3_prtx_s cn58xx;
	struct cvmx_pip_stat3_prtx_s cn58xxp1;
	struct cvmx_pip_stat3_prtx_s cn61xx;
	struct cvmx_pip_stat3_prtx_s cn63xx;
	struct cvmx_pip_stat3_prtx_s cn63xxp1;
	struct cvmx_pip_stat3_prtx_s cn66xx;
	struct cvmx_pip_stat3_prtx_s cn70xx;
	struct cvmx_pip_stat3_prtx_s cn70xxp1;
	struct cvmx_pip_stat3_prtx_s cnf71xx;
};

typedef union cvmx_pip_stat3_prtx cvmx_pip_stat3_prtx_t;

/**
 * cvmx_pip_stat4_#
 *
 * PIP_STAT4_X = PIP_STAT_HIST1    / PIP_STAT_HIST0
 *
 */
union cvmx_pip_stat4_x {
	u64 u64;
	struct cvmx_pip_stat4_x_s {
		u64 h65to127 : 32;
		u64 h64 : 32;
	} s;
	struct cvmx_pip_stat4_x_s cn68xx;
	struct cvmx_pip_stat4_x_s cn68xxp1;
};

typedef union cvmx_pip_stat4_x cvmx_pip_stat4_x_t;

/**
 * cvmx_pip_stat4_prt#
 *
 * PIP_STAT4_PRTX = PIP_STAT_HIST1    / PIP_STAT_HIST0
 *
 */
union cvmx_pip_stat4_prtx {
	u64 u64;
	struct cvmx_pip_stat4_prtx_s {
		u64 h65to127 : 32;
		u64 h64 : 32;
	} s;
	struct cvmx_pip_stat4_prtx_s cn30xx;
	struct cvmx_pip_stat4_prtx_s cn31xx;
	struct cvmx_pip_stat4_prtx_s cn38xx;
	struct cvmx_pip_stat4_prtx_s cn38xxp2;
	struct cvmx_pip_stat4_prtx_s cn50xx;
	struct cvmx_pip_stat4_prtx_s cn52xx;
	struct cvmx_pip_stat4_prtx_s cn52xxp1;
	struct cvmx_pip_stat4_prtx_s cn56xx;
	struct cvmx_pip_stat4_prtx_s cn56xxp1;
	struct cvmx_pip_stat4_prtx_s cn58xx;
	struct cvmx_pip_stat4_prtx_s cn58xxp1;
	struct cvmx_pip_stat4_prtx_s cn61xx;
	struct cvmx_pip_stat4_prtx_s cn63xx;
	struct cvmx_pip_stat4_prtx_s cn63xxp1;
	struct cvmx_pip_stat4_prtx_s cn66xx;
	struct cvmx_pip_stat4_prtx_s cn70xx;
	struct cvmx_pip_stat4_prtx_s cn70xxp1;
	struct cvmx_pip_stat4_prtx_s cnf71xx;
};

typedef union cvmx_pip_stat4_prtx cvmx_pip_stat4_prtx_t;

/**
 * cvmx_pip_stat5_#
 *
 * PIP_STAT5_X = PIP_STAT_HIST3    / PIP_STAT_HIST2
 *
 */
union cvmx_pip_stat5_x {
	u64 u64;
	struct cvmx_pip_stat5_x_s {
		u64 h256to511 : 32;
		u64 h128to255 : 32;
	} s;
	struct cvmx_pip_stat5_x_s cn68xx;
	struct cvmx_pip_stat5_x_s cn68xxp1;
};

typedef union cvmx_pip_stat5_x cvmx_pip_stat5_x_t;

/**
 * cvmx_pip_stat5_prt#
 *
 * PIP_STAT5_PRTX = PIP_STAT_HIST3    / PIP_STAT_HIST2
 *
 */
union cvmx_pip_stat5_prtx {
	u64 u64;
	struct cvmx_pip_stat5_prtx_s {
		u64 h256to511 : 32;
		u64 h128to255 : 32;
	} s;
	struct cvmx_pip_stat5_prtx_s cn30xx;
	struct cvmx_pip_stat5_prtx_s cn31xx;
	struct cvmx_pip_stat5_prtx_s cn38xx;
	struct cvmx_pip_stat5_prtx_s cn38xxp2;
	struct cvmx_pip_stat5_prtx_s cn50xx;
	struct cvmx_pip_stat5_prtx_s cn52xx;
	struct cvmx_pip_stat5_prtx_s cn52xxp1;
	struct cvmx_pip_stat5_prtx_s cn56xx;
	struct cvmx_pip_stat5_prtx_s cn56xxp1;
	struct cvmx_pip_stat5_prtx_s cn58xx;
	struct cvmx_pip_stat5_prtx_s cn58xxp1;
	struct cvmx_pip_stat5_prtx_s cn61xx;
	struct cvmx_pip_stat5_prtx_s cn63xx;
	struct cvmx_pip_stat5_prtx_s cn63xxp1;
	struct cvmx_pip_stat5_prtx_s cn66xx;
	struct cvmx_pip_stat5_prtx_s cn70xx;
	struct cvmx_pip_stat5_prtx_s cn70xxp1;
	struct cvmx_pip_stat5_prtx_s cnf71xx;
};

typedef union cvmx_pip_stat5_prtx cvmx_pip_stat5_prtx_t;

/**
 * cvmx_pip_stat6_#
 *
 * PIP_STAT6_X = PIP_STAT_HIST5    / PIP_STAT_HIST4
 *
 */
union cvmx_pip_stat6_x {
	u64 u64;
	struct cvmx_pip_stat6_x_s {
		u64 h1024to1518 : 32;
		u64 h512to1023 : 32;
	} s;
	struct cvmx_pip_stat6_x_s cn68xx;
	struct cvmx_pip_stat6_x_s cn68xxp1;
};

typedef union cvmx_pip_stat6_x cvmx_pip_stat6_x_t;

/**
 * cvmx_pip_stat6_prt#
 *
 * PIP_STAT6_PRTX = PIP_STAT_HIST5    / PIP_STAT_HIST4
 *
 */
union cvmx_pip_stat6_prtx {
	u64 u64;
	struct cvmx_pip_stat6_prtx_s {
		u64 h1024to1518 : 32;
		u64 h512to1023 : 32;
	} s;
	struct cvmx_pip_stat6_prtx_s cn30xx;
	struct cvmx_pip_stat6_prtx_s cn31xx;
	struct cvmx_pip_stat6_prtx_s cn38xx;
	struct cvmx_pip_stat6_prtx_s cn38xxp2;
	struct cvmx_pip_stat6_prtx_s cn50xx;
	struct cvmx_pip_stat6_prtx_s cn52xx;
	struct cvmx_pip_stat6_prtx_s cn52xxp1;
	struct cvmx_pip_stat6_prtx_s cn56xx;
	struct cvmx_pip_stat6_prtx_s cn56xxp1;
	struct cvmx_pip_stat6_prtx_s cn58xx;
	struct cvmx_pip_stat6_prtx_s cn58xxp1;
	struct cvmx_pip_stat6_prtx_s cn61xx;
	struct cvmx_pip_stat6_prtx_s cn63xx;
	struct cvmx_pip_stat6_prtx_s cn63xxp1;
	struct cvmx_pip_stat6_prtx_s cn66xx;
	struct cvmx_pip_stat6_prtx_s cn70xx;
	struct cvmx_pip_stat6_prtx_s cn70xxp1;
	struct cvmx_pip_stat6_prtx_s cnf71xx;
};

typedef union cvmx_pip_stat6_prtx cvmx_pip_stat6_prtx_t;

/**
 * cvmx_pip_stat7_#
 *
 * PIP_STAT7_X = PIP_STAT_FCS      / PIP_STAT_HIST6
 *
 */
union cvmx_pip_stat7_x {
	u64 u64;
	struct cvmx_pip_stat7_x_s {
		u64 fcs : 32;
		u64 h1519 : 32;
	} s;
	struct cvmx_pip_stat7_x_s cn68xx;
	struct cvmx_pip_stat7_x_s cn68xxp1;
};

typedef union cvmx_pip_stat7_x cvmx_pip_stat7_x_t;

/**
 * cvmx_pip_stat7_prt#
 *
 * PIP_STAT7_PRTX = PIP_STAT_FCS      / PIP_STAT_HIST6
 *
 *
 * Notes:
 * DPI does not check FCS, therefore FCS will never increment on DPI ports 32-35
 * sRIO does not check FCS, therefore FCS will never increment on sRIO ports 40-47
 */
union cvmx_pip_stat7_prtx {
	u64 u64;
	struct cvmx_pip_stat7_prtx_s {
		u64 fcs : 32;
		u64 h1519 : 32;
	} s;
	struct cvmx_pip_stat7_prtx_s cn30xx;
	struct cvmx_pip_stat7_prtx_s cn31xx;
	struct cvmx_pip_stat7_prtx_s cn38xx;
	struct cvmx_pip_stat7_prtx_s cn38xxp2;
	struct cvmx_pip_stat7_prtx_s cn50xx;
	struct cvmx_pip_stat7_prtx_s cn52xx;
	struct cvmx_pip_stat7_prtx_s cn52xxp1;
	struct cvmx_pip_stat7_prtx_s cn56xx;
	struct cvmx_pip_stat7_prtx_s cn56xxp1;
	struct cvmx_pip_stat7_prtx_s cn58xx;
	struct cvmx_pip_stat7_prtx_s cn58xxp1;
	struct cvmx_pip_stat7_prtx_s cn61xx;
	struct cvmx_pip_stat7_prtx_s cn63xx;
	struct cvmx_pip_stat7_prtx_s cn63xxp1;
	struct cvmx_pip_stat7_prtx_s cn66xx;
	struct cvmx_pip_stat7_prtx_s cn70xx;
	struct cvmx_pip_stat7_prtx_s cn70xxp1;
	struct cvmx_pip_stat7_prtx_s cnf71xx;
};

typedef union cvmx_pip_stat7_prtx cvmx_pip_stat7_prtx_t;

/**
 * cvmx_pip_stat8_#
 *
 * PIP_STAT8_X = PIP_STAT_FRAG     / PIP_STAT_UNDER
 *
 */
union cvmx_pip_stat8_x {
	u64 u64;
	struct cvmx_pip_stat8_x_s {
		u64 frag : 32;
		u64 undersz : 32;
	} s;
	struct cvmx_pip_stat8_x_s cn68xx;
	struct cvmx_pip_stat8_x_s cn68xxp1;
};

typedef union cvmx_pip_stat8_x cvmx_pip_stat8_x_t;

/**
 * cvmx_pip_stat8_prt#
 *
 * PIP_STAT8_PRTX = PIP_STAT_FRAG     / PIP_STAT_UNDER
 *
 *
 * Notes:
 * DPI does not check FCS, therefore FRAG will never increment on DPI ports 32-35
 * sRIO does not check FCS, therefore FRAG will never increment on sRIO ports 40-47
 */
union cvmx_pip_stat8_prtx {
	u64 u64;
	struct cvmx_pip_stat8_prtx_s {
		u64 frag : 32;
		u64 undersz : 32;
	} s;
	struct cvmx_pip_stat8_prtx_s cn30xx;
	struct cvmx_pip_stat8_prtx_s cn31xx;
	struct cvmx_pip_stat8_prtx_s cn38xx;
	struct cvmx_pip_stat8_prtx_s cn38xxp2;
	struct cvmx_pip_stat8_prtx_s cn50xx;
	struct cvmx_pip_stat8_prtx_s cn52xx;
	struct cvmx_pip_stat8_prtx_s cn52xxp1;
	struct cvmx_pip_stat8_prtx_s cn56xx;
	struct cvmx_pip_stat8_prtx_s cn56xxp1;
	struct cvmx_pip_stat8_prtx_s cn58xx;
	struct cvmx_pip_stat8_prtx_s cn58xxp1;
	struct cvmx_pip_stat8_prtx_s cn61xx;
	struct cvmx_pip_stat8_prtx_s cn63xx;
	struct cvmx_pip_stat8_prtx_s cn63xxp1;
	struct cvmx_pip_stat8_prtx_s cn66xx;
	struct cvmx_pip_stat8_prtx_s cn70xx;
	struct cvmx_pip_stat8_prtx_s cn70xxp1;
	struct cvmx_pip_stat8_prtx_s cnf71xx;
};

typedef union cvmx_pip_stat8_prtx cvmx_pip_stat8_prtx_t;

/**
 * cvmx_pip_stat9_#
 *
 * PIP_STAT9_X = PIP_STAT_JABBER   / PIP_STAT_OVER
 *
 */
union cvmx_pip_stat9_x {
	u64 u64;
	struct cvmx_pip_stat9_x_s {
		u64 jabber : 32;
		u64 oversz : 32;
	} s;
	struct cvmx_pip_stat9_x_s cn68xx;
	struct cvmx_pip_stat9_x_s cn68xxp1;
};

typedef union cvmx_pip_stat9_x cvmx_pip_stat9_x_t;

/**
 * cvmx_pip_stat9_prt#
 *
 * PIP_STAT9_PRTX = PIP_STAT_JABBER   / PIP_STAT_OVER
 *
 *
 * Notes:
 * DPI does not check FCS, therefore JABBER will never increment on DPI ports 32-35
 * sRIO does not check FCS, therefore JABBER will never increment on sRIO ports 40-47 due to FCS errors
 * sRIO does use the JABBER opcode to communicate sRIO error, therefore JABBER can increment under the sRIO error conditions
 */
union cvmx_pip_stat9_prtx {
	u64 u64;
	struct cvmx_pip_stat9_prtx_s {
		u64 jabber : 32;
		u64 oversz : 32;
	} s;
	struct cvmx_pip_stat9_prtx_s cn30xx;
	struct cvmx_pip_stat9_prtx_s cn31xx;
	struct cvmx_pip_stat9_prtx_s cn38xx;
	struct cvmx_pip_stat9_prtx_s cn38xxp2;
	struct cvmx_pip_stat9_prtx_s cn50xx;
	struct cvmx_pip_stat9_prtx_s cn52xx;
	struct cvmx_pip_stat9_prtx_s cn52xxp1;
	struct cvmx_pip_stat9_prtx_s cn56xx;
	struct cvmx_pip_stat9_prtx_s cn56xxp1;
	struct cvmx_pip_stat9_prtx_s cn58xx;
	struct cvmx_pip_stat9_prtx_s cn58xxp1;
	struct cvmx_pip_stat9_prtx_s cn61xx;
	struct cvmx_pip_stat9_prtx_s cn63xx;
	struct cvmx_pip_stat9_prtx_s cn63xxp1;
	struct cvmx_pip_stat9_prtx_s cn66xx;
	struct cvmx_pip_stat9_prtx_s cn70xx;
	struct cvmx_pip_stat9_prtx_s cn70xxp1;
	struct cvmx_pip_stat9_prtx_s cnf71xx;
};

typedef union cvmx_pip_stat9_prtx cvmx_pip_stat9_prtx_t;

/**
 * cvmx_pip_stat_ctl
 *
 * Controls how the PIP statistics counters are handled.
 *
 */
union cvmx_pip_stat_ctl {
	u64 u64;
	struct cvmx_pip_stat_ctl_s {
		u64 reserved_9_63 : 55;
		u64 mode : 1;
		u64 reserved_1_7 : 7;
		u64 rdclr : 1;
	} s;
	struct cvmx_pip_stat_ctl_cn30xx {
		u64 reserved_1_63 : 63;
		u64 rdclr : 1;
	} cn30xx;
	struct cvmx_pip_stat_ctl_cn30xx cn31xx;
	struct cvmx_pip_stat_ctl_cn30xx cn38xx;
	struct cvmx_pip_stat_ctl_cn30xx cn38xxp2;
	struct cvmx_pip_stat_ctl_cn30xx cn50xx;
	struct cvmx_pip_stat_ctl_cn30xx cn52xx;
	struct cvmx_pip_stat_ctl_cn30xx cn52xxp1;
	struct cvmx_pip_stat_ctl_cn30xx cn56xx;
	struct cvmx_pip_stat_ctl_cn30xx cn56xxp1;
	struct cvmx_pip_stat_ctl_cn30xx cn58xx;
	struct cvmx_pip_stat_ctl_cn30xx cn58xxp1;
	struct cvmx_pip_stat_ctl_cn30xx cn61xx;
	struct cvmx_pip_stat_ctl_cn30xx cn63xx;
	struct cvmx_pip_stat_ctl_cn30xx cn63xxp1;
	struct cvmx_pip_stat_ctl_cn30xx cn66xx;
	struct cvmx_pip_stat_ctl_s cn68xx;
	struct cvmx_pip_stat_ctl_s cn68xxp1;
	struct cvmx_pip_stat_ctl_cn30xx cn70xx;
	struct cvmx_pip_stat_ctl_cn30xx cn70xxp1;
	struct cvmx_pip_stat_ctl_cn30xx cnf71xx;
};

typedef union cvmx_pip_stat_ctl cvmx_pip_stat_ctl_t;

/**
 * cvmx_pip_stat_inb_errs#
 *
 * Inbound stats collect all data sent to PIP from all packet interfaces.
 * Its the raw counts of everything that comes into the block.  The counts
 * will reflect all error packets and packets dropped by the PKI RED engine.
 * These counts are intended for system debug, but could convey useful
 * information in production systems.
 */
union cvmx_pip_stat_inb_errsx {
	u64 u64;
	struct cvmx_pip_stat_inb_errsx_s {
		u64 reserved_16_63 : 48;
		u64 errs : 16;
	} s;
	struct cvmx_pip_stat_inb_errsx_s cn30xx;
	struct cvmx_pip_stat_inb_errsx_s cn31xx;
	struct cvmx_pip_stat_inb_errsx_s cn38xx;
	struct cvmx_pip_stat_inb_errsx_s cn38xxp2;
	struct cvmx_pip_stat_inb_errsx_s cn50xx;
	struct cvmx_pip_stat_inb_errsx_s cn52xx;
	struct cvmx_pip_stat_inb_errsx_s cn52xxp1;
	struct cvmx_pip_stat_inb_errsx_s cn56xx;
	struct cvmx_pip_stat_inb_errsx_s cn56xxp1;
	struct cvmx_pip_stat_inb_errsx_s cn58xx;
	struct cvmx_pip_stat_inb_errsx_s cn58xxp1;
	struct cvmx_pip_stat_inb_errsx_s cn61xx;
	struct cvmx_pip_stat_inb_errsx_s cn63xx;
	struct cvmx_pip_stat_inb_errsx_s cn63xxp1;
	struct cvmx_pip_stat_inb_errsx_s cn66xx;
	struct cvmx_pip_stat_inb_errsx_s cn70xx;
	struct cvmx_pip_stat_inb_errsx_s cn70xxp1;
	struct cvmx_pip_stat_inb_errsx_s cnf71xx;
};

typedef union cvmx_pip_stat_inb_errsx cvmx_pip_stat_inb_errsx_t;

/**
 * cvmx_pip_stat_inb_errs_pknd#
 *
 * PIP_STAT_INB_ERRS_PKNDX = Inbound error packets received by PIP per pkind
 *
 * Inbound stats collect all data sent to PIP from all packet interfaces.
 * Its the raw counts of everything that comes into the block.  The counts
 * will reflect all error packets and packets dropped by the PKI RED engine.
 * These counts are intended for system debug, but could convey useful
 * information in production systems.
 */
union cvmx_pip_stat_inb_errs_pkndx {
	u64 u64;
	struct cvmx_pip_stat_inb_errs_pkndx_s {
		u64 reserved_16_63 : 48;
		u64 errs : 16;
	} s;
	struct cvmx_pip_stat_inb_errs_pkndx_s cn68xx;
	struct cvmx_pip_stat_inb_errs_pkndx_s cn68xxp1;
};

typedef union cvmx_pip_stat_inb_errs_pkndx cvmx_pip_stat_inb_errs_pkndx_t;

/**
 * cvmx_pip_stat_inb_octs#
 *
 * Inbound stats collect all data sent to PIP from all packet interfaces.
 * Its the raw counts of everything that comes into the block.  The counts
 * will reflect all error packets and packets dropped by the PKI RED engine.
 * These counts are intended for system debug, but could convey useful
 * information in production systems. The OCTS will include the bytes from
 * timestamp fields in PTP_MODE.
 */
union cvmx_pip_stat_inb_octsx {
	u64 u64;
	struct cvmx_pip_stat_inb_octsx_s {
		u64 reserved_48_63 : 16;
		u64 octs : 48;
	} s;
	struct cvmx_pip_stat_inb_octsx_s cn30xx;
	struct cvmx_pip_stat_inb_octsx_s cn31xx;
	struct cvmx_pip_stat_inb_octsx_s cn38xx;
	struct cvmx_pip_stat_inb_octsx_s cn38xxp2;
	struct cvmx_pip_stat_inb_octsx_s cn50xx;
	struct cvmx_pip_stat_inb_octsx_s cn52xx;
	struct cvmx_pip_stat_inb_octsx_s cn52xxp1;
	struct cvmx_pip_stat_inb_octsx_s cn56xx;
	struct cvmx_pip_stat_inb_octsx_s cn56xxp1;
	struct cvmx_pip_stat_inb_octsx_s cn58xx;
	struct cvmx_pip_stat_inb_octsx_s cn58xxp1;
	struct cvmx_pip_stat_inb_octsx_s cn61xx;
	struct cvmx_pip_stat_inb_octsx_s cn63xx;
	struct cvmx_pip_stat_inb_octsx_s cn63xxp1;
	struct cvmx_pip_stat_inb_octsx_s cn66xx;
	struct cvmx_pip_stat_inb_octsx_s cn70xx;
	struct cvmx_pip_stat_inb_octsx_s cn70xxp1;
	struct cvmx_pip_stat_inb_octsx_s cnf71xx;
};

typedef union cvmx_pip_stat_inb_octsx cvmx_pip_stat_inb_octsx_t;

/**
 * cvmx_pip_stat_inb_octs_pknd#
 *
 * PIP_STAT_INB_OCTS_PKNDX = Inbound octets received by PIP per pkind
 *
 * Inbound stats collect all data sent to PIP from all packet interfaces.
 * Its the raw counts of everything that comes into the block.  The counts
 * will reflect all error packets and packets dropped by the PKI RED engine.
 * These counts are intended for system debug, but could convey useful
 * information in production systems. The OCTS will include the bytes from
 * timestamp fields in PTP_MODE.
 */
union cvmx_pip_stat_inb_octs_pkndx {
	u64 u64;
	struct cvmx_pip_stat_inb_octs_pkndx_s {
		u64 reserved_48_63 : 16;
		u64 octs : 48;
	} s;
	struct cvmx_pip_stat_inb_octs_pkndx_s cn68xx;
	struct cvmx_pip_stat_inb_octs_pkndx_s cn68xxp1;
};

typedef union cvmx_pip_stat_inb_octs_pkndx cvmx_pip_stat_inb_octs_pkndx_t;

/**
 * cvmx_pip_stat_inb_pkts#
 *
 * Inbound stats collect all data sent to PIP from all packet interfaces.
 * Its the raw counts of everything that comes into the block.  The counts
 * will reflect all error packets and packets dropped by the PKI RED engine.
 * These counts are intended for system debug, but could convey useful
 * information in production systems.
 */
union cvmx_pip_stat_inb_pktsx {
	u64 u64;
	struct cvmx_pip_stat_inb_pktsx_s {
		u64 reserved_32_63 : 32;
		u64 pkts : 32;
	} s;
	struct cvmx_pip_stat_inb_pktsx_s cn30xx;
	struct cvmx_pip_stat_inb_pktsx_s cn31xx;
	struct cvmx_pip_stat_inb_pktsx_s cn38xx;
	struct cvmx_pip_stat_inb_pktsx_s cn38xxp2;
	struct cvmx_pip_stat_inb_pktsx_s cn50xx;
	struct cvmx_pip_stat_inb_pktsx_s cn52xx;
	struct cvmx_pip_stat_inb_pktsx_s cn52xxp1;
	struct cvmx_pip_stat_inb_pktsx_s cn56xx;
	struct cvmx_pip_stat_inb_pktsx_s cn56xxp1;
	struct cvmx_pip_stat_inb_pktsx_s cn58xx;
	struct cvmx_pip_stat_inb_pktsx_s cn58xxp1;
	struct cvmx_pip_stat_inb_pktsx_s cn61xx;
	struct cvmx_pip_stat_inb_pktsx_s cn63xx;
	struct cvmx_pip_stat_inb_pktsx_s cn63xxp1;
	struct cvmx_pip_stat_inb_pktsx_s cn66xx;
	struct cvmx_pip_stat_inb_pktsx_s cn70xx;
	struct cvmx_pip_stat_inb_pktsx_s cn70xxp1;
	struct cvmx_pip_stat_inb_pktsx_s cnf71xx;
};

typedef union cvmx_pip_stat_inb_pktsx cvmx_pip_stat_inb_pktsx_t;

/**
 * cvmx_pip_stat_inb_pkts_pknd#
 *
 * PIP_STAT_INB_PKTS_PKNDX = Inbound packets received by PIP per pkind
 *
 * Inbound stats collect all data sent to PIP from all packet interfaces.
 * Its the raw counts of everything that comes into the block.  The counts
 * will reflect all error packets and packets dropped by the PKI RED engine.
 * These counts are intended for system debug, but could convey useful
 * information in production systems.
 */
union cvmx_pip_stat_inb_pkts_pkndx {
	u64 u64;
	struct cvmx_pip_stat_inb_pkts_pkndx_s {
		u64 reserved_32_63 : 32;
		u64 pkts : 32;
	} s;
	struct cvmx_pip_stat_inb_pkts_pkndx_s cn68xx;
	struct cvmx_pip_stat_inb_pkts_pkndx_s cn68xxp1;
};

typedef union cvmx_pip_stat_inb_pkts_pkndx cvmx_pip_stat_inb_pkts_pkndx_t;

/**
 * cvmx_pip_sub_pkind_fcs#
 */
union cvmx_pip_sub_pkind_fcsx {
	u64 u64;
	struct cvmx_pip_sub_pkind_fcsx_s {
		u64 port_bit : 64;
	} s;
	struct cvmx_pip_sub_pkind_fcsx_s cn68xx;
	struct cvmx_pip_sub_pkind_fcsx_s cn68xxp1;
};

typedef union cvmx_pip_sub_pkind_fcsx cvmx_pip_sub_pkind_fcsx_t;

/**
 * cvmx_pip_tag_inc#
 *
 * # $PIP_TAG_INCX = 0x300+X X=(0..63) RegType=(RSL) RtlReg=(pip_tag_inc_csr_direct_TestbuilderTask)
 *
 */
union cvmx_pip_tag_incx {
	u64 u64;
	struct cvmx_pip_tag_incx_s {
		u64 reserved_8_63 : 56;
		u64 en : 8;
	} s;
	struct cvmx_pip_tag_incx_s cn30xx;
	struct cvmx_pip_tag_incx_s cn31xx;
	struct cvmx_pip_tag_incx_s cn38xx;
	struct cvmx_pip_tag_incx_s cn38xxp2;
	struct cvmx_pip_tag_incx_s cn50xx;
	struct cvmx_pip_tag_incx_s cn52xx;
	struct cvmx_pip_tag_incx_s cn52xxp1;
	struct cvmx_pip_tag_incx_s cn56xx;
	struct cvmx_pip_tag_incx_s cn56xxp1;
	struct cvmx_pip_tag_incx_s cn58xx;
	struct cvmx_pip_tag_incx_s cn58xxp1;
	struct cvmx_pip_tag_incx_s cn61xx;
	struct cvmx_pip_tag_incx_s cn63xx;
	struct cvmx_pip_tag_incx_s cn63xxp1;
	struct cvmx_pip_tag_incx_s cn66xx;
	struct cvmx_pip_tag_incx_s cn68xx;
	struct cvmx_pip_tag_incx_s cn68xxp1;
	struct cvmx_pip_tag_incx_s cn70xx;
	struct cvmx_pip_tag_incx_s cn70xxp1;
	struct cvmx_pip_tag_incx_s cnf71xx;
};

typedef union cvmx_pip_tag_incx cvmx_pip_tag_incx_t;

/**
 * cvmx_pip_tag_mask
 *
 * PIP_TAG_MASK = Mask bit in the tag generation
 *
 */
union cvmx_pip_tag_mask {
	u64 u64;
	struct cvmx_pip_tag_mask_s {
		u64 reserved_16_63 : 48;
		u64 mask : 16;
	} s;
	struct cvmx_pip_tag_mask_s cn30xx;
	struct cvmx_pip_tag_mask_s cn31xx;
	struct cvmx_pip_tag_mask_s cn38xx;
	struct cvmx_pip_tag_mask_s cn38xxp2;
	struct cvmx_pip_tag_mask_s cn50xx;
	struct cvmx_pip_tag_mask_s cn52xx;
	struct cvmx_pip_tag_mask_s cn52xxp1;
	struct cvmx_pip_tag_mask_s cn56xx;
	struct cvmx_pip_tag_mask_s cn56xxp1;
	struct cvmx_pip_tag_mask_s cn58xx;
	struct cvmx_pip_tag_mask_s cn58xxp1;
	struct cvmx_pip_tag_mask_s cn61xx;
	struct cvmx_pip_tag_mask_s cn63xx;
	struct cvmx_pip_tag_mask_s cn63xxp1;
	struct cvmx_pip_tag_mask_s cn66xx;
	struct cvmx_pip_tag_mask_s cn68xx;
	struct cvmx_pip_tag_mask_s cn68xxp1;
	struct cvmx_pip_tag_mask_s cn70xx;
	struct cvmx_pip_tag_mask_s cn70xxp1;
	struct cvmx_pip_tag_mask_s cnf71xx;
};

typedef union cvmx_pip_tag_mask cvmx_pip_tag_mask_t;

/**
 * cvmx_pip_tag_secret
 *
 * The source and destination IV's provide a mechanism for each Octeon to be unique.
 *
 */
union cvmx_pip_tag_secret {
	u64 u64;
	struct cvmx_pip_tag_secret_s {
		u64 reserved_32_63 : 32;
		u64 dst : 16;
		u64 src : 16;
	} s;
	struct cvmx_pip_tag_secret_s cn30xx;
	struct cvmx_pip_tag_secret_s cn31xx;
	struct cvmx_pip_tag_secret_s cn38xx;
	struct cvmx_pip_tag_secret_s cn38xxp2;
	struct cvmx_pip_tag_secret_s cn50xx;
	struct cvmx_pip_tag_secret_s cn52xx;
	struct cvmx_pip_tag_secret_s cn52xxp1;
	struct cvmx_pip_tag_secret_s cn56xx;
	struct cvmx_pip_tag_secret_s cn56xxp1;
	struct cvmx_pip_tag_secret_s cn58xx;
	struct cvmx_pip_tag_secret_s cn58xxp1;
	struct cvmx_pip_tag_secret_s cn61xx;
	struct cvmx_pip_tag_secret_s cn63xx;
	struct cvmx_pip_tag_secret_s cn63xxp1;
	struct cvmx_pip_tag_secret_s cn66xx;
	struct cvmx_pip_tag_secret_s cn68xx;
	struct cvmx_pip_tag_secret_s cn68xxp1;
	struct cvmx_pip_tag_secret_s cn70xx;
	struct cvmx_pip_tag_secret_s cn70xxp1;
	struct cvmx_pip_tag_secret_s cnf71xx;
};

typedef union cvmx_pip_tag_secret cvmx_pip_tag_secret_t;

/**
 * cvmx_pip_todo_entry
 *
 * Summary of the current packet that has completed and waiting to be processed
 *
 */
union cvmx_pip_todo_entry {
	u64 u64;
	struct cvmx_pip_todo_entry_s {
		u64 val : 1;
		u64 reserved_62_62 : 1;
		u64 entry : 62;
	} s;
	struct cvmx_pip_todo_entry_s cn30xx;
	struct cvmx_pip_todo_entry_s cn31xx;
	struct cvmx_pip_todo_entry_s cn38xx;
	struct cvmx_pip_todo_entry_s cn38xxp2;
	struct cvmx_pip_todo_entry_s cn50xx;
	struct cvmx_pip_todo_entry_s cn52xx;
	struct cvmx_pip_todo_entry_s cn52xxp1;
	struct cvmx_pip_todo_entry_s cn56xx;
	struct cvmx_pip_todo_entry_s cn56xxp1;
	struct cvmx_pip_todo_entry_s cn58xx;
	struct cvmx_pip_todo_entry_s cn58xxp1;
	struct cvmx_pip_todo_entry_s cn61xx;
	struct cvmx_pip_todo_entry_s cn63xx;
	struct cvmx_pip_todo_entry_s cn63xxp1;
	struct cvmx_pip_todo_entry_s cn66xx;
	struct cvmx_pip_todo_entry_s cn68xx;
	struct cvmx_pip_todo_entry_s cn68xxp1;
	struct cvmx_pip_todo_entry_s cn70xx;
	struct cvmx_pip_todo_entry_s cn70xxp1;
	struct cvmx_pip_todo_entry_s cnf71xx;
};

typedef union cvmx_pip_todo_entry cvmx_pip_todo_entry_t;

/**
 * cvmx_pip_vlan_etypes#
 */
union cvmx_pip_vlan_etypesx {
	u64 u64;
	struct cvmx_pip_vlan_etypesx_s {
		u64 type3 : 16;
		u64 type2 : 16;
		u64 type1 : 16;
		u64 type0 : 16;
	} s;
	struct cvmx_pip_vlan_etypesx_s cn61xx;
	struct cvmx_pip_vlan_etypesx_s cn66xx;
	struct cvmx_pip_vlan_etypesx_s cn68xx;
	struct cvmx_pip_vlan_etypesx_s cn70xx;
	struct cvmx_pip_vlan_etypesx_s cn70xxp1;
	struct cvmx_pip_vlan_etypesx_s cnf71xx;
};

typedef union cvmx_pip_vlan_etypesx cvmx_pip_vlan_etypesx_t;

/**
 * cvmx_pip_xstat0_prt#
 *
 * PIP_XSTAT0_PRT = PIP_XSTAT_DRP_PKTS / PIP_XSTAT_DRP_OCTS
 *
 */
union cvmx_pip_xstat0_prtx {
	u64 u64;
	struct cvmx_pip_xstat0_prtx_s {
		u64 drp_pkts : 32;
		u64 drp_octs : 32;
	} s;
	struct cvmx_pip_xstat0_prtx_s cn63xx;
	struct cvmx_pip_xstat0_prtx_s cn63xxp1;
	struct cvmx_pip_xstat0_prtx_s cn66xx;
};

typedef union cvmx_pip_xstat0_prtx cvmx_pip_xstat0_prtx_t;

/**
 * cvmx_pip_xstat10_prt#
 *
 * PIP_XSTAT10_PRTX = PIP_XSTAT_L2_MCAST / PIP_XSTAT_L2_BCAST
 *
 */
union cvmx_pip_xstat10_prtx {
	u64 u64;
	struct cvmx_pip_xstat10_prtx_s {
		u64 bcast : 32;
		u64 mcast : 32;
	} s;
	struct cvmx_pip_xstat10_prtx_s cn63xx;
	struct cvmx_pip_xstat10_prtx_s cn63xxp1;
	struct cvmx_pip_xstat10_prtx_s cn66xx;
};

typedef union cvmx_pip_xstat10_prtx cvmx_pip_xstat10_prtx_t;

/**
 * cvmx_pip_xstat11_prt#
 *
 * PIP_XSTAT11_PRTX = PIP_XSTAT_L3_MCAST / PIP_XSTAT_L3_BCAST
 *
 */
union cvmx_pip_xstat11_prtx {
	u64 u64;
	struct cvmx_pip_xstat11_prtx_s {
		u64 bcast : 32;
		u64 mcast : 32;
	} s;
	struct cvmx_pip_xstat11_prtx_s cn63xx;
	struct cvmx_pip_xstat11_prtx_s cn63xxp1;
	struct cvmx_pip_xstat11_prtx_s cn66xx;
};

typedef union cvmx_pip_xstat11_prtx cvmx_pip_xstat11_prtx_t;

/**
 * cvmx_pip_xstat1_prt#
 *
 * PIP_XSTAT1_PRTX = PIP_XSTAT_OCTS
 *
 */
union cvmx_pip_xstat1_prtx {
	u64 u64;
	struct cvmx_pip_xstat1_prtx_s {
		u64 reserved_48_63 : 16;
		u64 octs : 48;
	} s;
	struct cvmx_pip_xstat1_prtx_s cn63xx;
	struct cvmx_pip_xstat1_prtx_s cn63xxp1;
	struct cvmx_pip_xstat1_prtx_s cn66xx;
};

typedef union cvmx_pip_xstat1_prtx cvmx_pip_xstat1_prtx_t;

/**
 * cvmx_pip_xstat2_prt#
 *
 * PIP_XSTAT2_PRTX = PIP_XSTAT_PKTS     / PIP_XSTAT_RAW
 *
 */
union cvmx_pip_xstat2_prtx {
	u64 u64;
	struct cvmx_pip_xstat2_prtx_s {
		u64 pkts : 32;
		u64 raw : 32;
	} s;
	struct cvmx_pip_xstat2_prtx_s cn63xx;
	struct cvmx_pip_xstat2_prtx_s cn63xxp1;
	struct cvmx_pip_xstat2_prtx_s cn66xx;
};

typedef union cvmx_pip_xstat2_prtx cvmx_pip_xstat2_prtx_t;

/**
 * cvmx_pip_xstat3_prt#
 *
 * PIP_XSTAT3_PRTX = PIP_XSTAT_BCST     / PIP_XSTAT_MCST
 *
 */
union cvmx_pip_xstat3_prtx {
	u64 u64;
	struct cvmx_pip_xstat3_prtx_s {
		u64 bcst : 32;
		u64 mcst : 32;
	} s;
	struct cvmx_pip_xstat3_prtx_s cn63xx;
	struct cvmx_pip_xstat3_prtx_s cn63xxp1;
	struct cvmx_pip_xstat3_prtx_s cn66xx;
};

typedef union cvmx_pip_xstat3_prtx cvmx_pip_xstat3_prtx_t;

/**
 * cvmx_pip_xstat4_prt#
 *
 * PIP_XSTAT4_PRTX = PIP_XSTAT_HIST1    / PIP_XSTAT_HIST0
 *
 */
union cvmx_pip_xstat4_prtx {
	u64 u64;
	struct cvmx_pip_xstat4_prtx_s {
		u64 h65to127 : 32;
		u64 h64 : 32;
	} s;
	struct cvmx_pip_xstat4_prtx_s cn63xx;
	struct cvmx_pip_xstat4_prtx_s cn63xxp1;
	struct cvmx_pip_xstat4_prtx_s cn66xx;
};

typedef union cvmx_pip_xstat4_prtx cvmx_pip_xstat4_prtx_t;

/**
 * cvmx_pip_xstat5_prt#
 *
 * PIP_XSTAT5_PRTX = PIP_XSTAT_HIST3    / PIP_XSTAT_HIST2
 *
 */
union cvmx_pip_xstat5_prtx {
	u64 u64;
	struct cvmx_pip_xstat5_prtx_s {
		u64 h256to511 : 32;
		u64 h128to255 : 32;
	} s;
	struct cvmx_pip_xstat5_prtx_s cn63xx;
	struct cvmx_pip_xstat5_prtx_s cn63xxp1;
	struct cvmx_pip_xstat5_prtx_s cn66xx;
};

typedef union cvmx_pip_xstat5_prtx cvmx_pip_xstat5_prtx_t;

/**
 * cvmx_pip_xstat6_prt#
 *
 * PIP_XSTAT6_PRTX = PIP_XSTAT_HIST5    / PIP_XSTAT_HIST4
 *
 */
union cvmx_pip_xstat6_prtx {
	u64 u64;
	struct cvmx_pip_xstat6_prtx_s {
		u64 h1024to1518 : 32;
		u64 h512to1023 : 32;
	} s;
	struct cvmx_pip_xstat6_prtx_s cn63xx;
	struct cvmx_pip_xstat6_prtx_s cn63xxp1;
	struct cvmx_pip_xstat6_prtx_s cn66xx;
};

typedef union cvmx_pip_xstat6_prtx cvmx_pip_xstat6_prtx_t;

/**
 * cvmx_pip_xstat7_prt#
 *
 * PIP_XSTAT7_PRTX = PIP_XSTAT_FCS      / PIP_XSTAT_HIST6
 *
 *
 * Notes:
 * DPI does not check FCS, therefore FCS will never increment on DPI ports 32-35
 * sRIO does not check FCS, therefore FCS will never increment on sRIO ports 40-47
 */
union cvmx_pip_xstat7_prtx {
	u64 u64;
	struct cvmx_pip_xstat7_prtx_s {
		u64 fcs : 32;
		u64 h1519 : 32;
	} s;
	struct cvmx_pip_xstat7_prtx_s cn63xx;
	struct cvmx_pip_xstat7_prtx_s cn63xxp1;
	struct cvmx_pip_xstat7_prtx_s cn66xx;
};

typedef union cvmx_pip_xstat7_prtx cvmx_pip_xstat7_prtx_t;

/**
 * cvmx_pip_xstat8_prt#
 *
 * PIP_XSTAT8_PRTX = PIP_XSTAT_FRAG     / PIP_XSTAT_UNDER
 *
 *
 * Notes:
 * DPI does not check FCS, therefore FRAG will never increment on DPI ports 32-35
 * sRIO does not check FCS, therefore FRAG will never increment on sRIO ports 40-47
 */
union cvmx_pip_xstat8_prtx {
	u64 u64;
	struct cvmx_pip_xstat8_prtx_s {
		u64 frag : 32;
		u64 undersz : 32;
	} s;
	struct cvmx_pip_xstat8_prtx_s cn63xx;
	struct cvmx_pip_xstat8_prtx_s cn63xxp1;
	struct cvmx_pip_xstat8_prtx_s cn66xx;
};

typedef union cvmx_pip_xstat8_prtx cvmx_pip_xstat8_prtx_t;

/**
 * cvmx_pip_xstat9_prt#
 *
 * PIP_XSTAT9_PRTX = PIP_XSTAT_JABBER   / PIP_XSTAT_OVER
 *
 *
 * Notes:
 * DPI does not check FCS, therefore JABBER will never increment on DPI ports 32-35
 * sRIO does not check FCS, therefore JABBER will never increment on sRIO ports 40-47 due to FCS errors
 * sRIO does use the JABBER opcode to communicate sRIO error, therefore JABBER can increment under the sRIO error conditions
 */
union cvmx_pip_xstat9_prtx {
	u64 u64;
	struct cvmx_pip_xstat9_prtx_s {
		u64 jabber : 32;
		u64 oversz : 32;
	} s;
	struct cvmx_pip_xstat9_prtx_s cn63xx;
	struct cvmx_pip_xstat9_prtx_s cn63xxp1;
	struct cvmx_pip_xstat9_prtx_s cn66xx;
};

typedef union cvmx_pip_xstat9_prtx cvmx_pip_xstat9_prtx_t;

#endif
