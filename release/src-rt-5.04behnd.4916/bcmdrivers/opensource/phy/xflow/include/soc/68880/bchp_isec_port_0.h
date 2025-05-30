/******************************************************************************
 * Copyright (C) 2021 Broadcom. The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * <:label-BRCM:2022:DUAL/GPL:standard
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 * 
 * :>
 *
 * Module Description:
 *
 *                      !!! DO NOT EDIT THIS FILE DIRECTLY !!!
 *
 * This module was generated automatically with RDB source input files.
 * You must edit the source file for changes to be made to this file.
 *
 * The launch point for all information concerning RDB is found at:
 *   http://confluence.broadcom.com/pages/viewpage.action?spaceKey=BCGRDB&title=RDB+%28Confluence%29+Home
 *
 * Date:             Generated on               Tue Nov 30 14:58:39 2021
 *                   Full Compile MD5 Checksum  76b8a32dee8db5f48c3a89e0a6278540
 *                     (minus title and desc)
 *                   MD5 Checksum               7dcf8d7f58b10ccb563b99b1683d60ae
 *
 * lock_release:     n/a
 *
 * Command Line:     /tools/dvtsw/current/Linux/combo_header.pl --multi --style=hydra --output_file=bchp /lwork/CPE/ni889281/bld5_1/depot/CommEngine/widgets/tools/hal_generator/BCM6888_A0_MACSEC/BCM6888_A0/config/ru_gen/BCM6888_A0.rdb
 *
 * Compiled with:    RDB Utility                combo_header.pl
 *                   RDB.pm                     3676
 *                   unknown                    unknown
 *                   Perl Interpreter           5.026000
 *                   Operating System           linux
 *                   Script Source              r_3688/Linux/combo_header.pl
 *                   DVTSWVER                   LOCAL r_3688/Linux/combo_header.pl
 *
 *
 *******************************************************************************/

#ifndef BCHP_ISEC_PORT_0_H__
#define BCHP_ISEC_PORT_0_H__

/***************************************************************************
 *ISEC_PORT_0
 ***************************************************************************/
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL            0x83700400 /* [RW][64] Per-Port ISEC control Register */
#define BCHP_ISEC_PORT_0_ISEC_RUD_MGMT_RULE_CTRL 0x8370040c /* [RW][32] Control register for the rules used in Rudimentry Management packet detection Register */
#define BCHP_ISEC_PORT_0_ISEC_AES_ICV_FAIL_CNT   0x83700414 /* [RO][32] ICV fail counter: Number of packets for ICV failure. Clear on read. Register */
#define BCHP_ISEC_PORT_0_ISEC_MTU_FAIL_CNT       0x8370041c /* [RO][32] MTU check fail counter: Number of MTU fail packets. Clear on read. Register */
#define BCHP_ISEC_PORT_0_ISEC_PDF_STATUS         0x83700424 /* [RW][32] Ingress parser-data FIFO status Register */
#define BCHP_ISEC_PORT_0_ISEC_PCF_BANK0_STATUS   0x8370042c /* [RW][32] Ingress parser-control FIFO Bank 0 status Register */
#define BCHP_ISEC_PORT_0_ISEC_PCF_BANK1_STATUS   0x83700434 /* [RW][32] Ingress parser-control FIFO Bank 1 status Register */
#define BCHP_ISEC_PORT_0_ISEC_CB_STR_FIFO_STATUS 0x8370043c /* [RW][32] Ingress internal Control Bus Storage FIFO status Register */
#define BCHP_ISEC_PORT_0_ISEC_IDF_STATUS         0x83700444 /* [RW][32] Ingress input-data FIFO status Register */
#define BCHP_ISEC_PORT_0_ISEC_ICF_STATUS         0x8370044c /* [RW][32] Ingress input-control FIFO status Register */
#define BCHP_ISEC_PORT_0_ISEC_ODF_STATUS         0x83700454 /* [RW][32] Ingress output-data FIFO status Register */
#define BCHP_ISEC_PORT_0_ISEC_TAG_FIFO_STATUS    0x8370045c /* [RW][32] Ingress tag-FIFO status Register */
#define BCHP_ISEC_PORT_0_ISEC_CW_FIFO_STATUS     0x83700464 /* [RW][32] Ingress CW FIFO status Register */

#define BCHP_ISEC_PORT_0_ISEC_RUD_MGMT_RULE_CTRL_WIDTH  32 /* [RW][32] */
#define BCHP_ISEC_PORT_0_ISEC_AES_ICV_FAIL_CNT_WIDTH    32 /* [RO][32] */
#define BCHP_ISEC_PORT_0_ISEC_MTU_FAIL_CNT_WIDTH        32 /* [RO][32] */
#define BCHP_ISEC_PORT_0_ISEC_PDF_STATUS_WIDTH          32 /* [RW][32] */
#define BCHP_ISEC_PORT_0_ISEC_PCF_BANK0_STATUS_WIDTH    32 /* [RW][32] */
#define BCHP_ISEC_PORT_0_ISEC_PCF_BANK1_STATUS_WIDTH    32 /* [RW][32] */
#define BCHP_ISEC_PORT_0_ISEC_CB_STR_FIFO_STATUS_WIDTH  32 /* [RW][32] */
#define BCHP_ISEC_PORT_0_ISEC_IDF_STATUS_WIDTH          32 /* [RW][32] */
#define BCHP_ISEC_PORT_0_ISEC_ICF_STATUS_WIDTH          32 /* [RW][32] */
#define BCHP_ISEC_PORT_0_ISEC_ODF_STATUS_WIDTH          32 /* [RW][32] */
#define BCHP_ISEC_PORT_0_ISEC_TAG_FIFO_STATUS_WIDTH     32 /* [RW][32] */
#define BCHP_ISEC_PORT_0_ISEC_CW_FIFO_STATUS_WIDTH      32 /* [RW][32] */

/***************************************************************************
 *ISEC_PP_CTRL - Per-Port ISEC control Register
 ***************************************************************************/
/* ISEC_PORT_0 :: ISEC_PP_CTRL :: reserved0 [63:54] */
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_reserved0_MASK               BCHP_UINT64_C(0xffc00000, 0x00000000)
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_reserved0_SHIFT              54

/* ISEC_PORT_0 :: ISEC_PP_CTRL :: MTU [53:40] */
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_MTU_MASK                     BCHP_UINT64_C(0x003fff00, 0x00000000)
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_MTU_SHIFT                    40
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_MTU_DEFAULT                  1514

/* ISEC_PORT_0 :: ISEC_PP_CTRL :: VXLAN_IPV6_W_UDP_SPTCAM_UDF_PAYLD_SEL [39:39] */
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_VXLAN_IPV6_W_UDP_SPTCAM_UDF_PAYLD_SEL_MASK BCHP_UINT64_C(0x00000080, 0x00000000)
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_VXLAN_IPV6_W_UDP_SPTCAM_UDF_PAYLD_SEL_SHIFT 39
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_VXLAN_IPV6_W_UDP_SPTCAM_UDF_PAYLD_SEL_DEFAULT 0

/* ISEC_PORT_0 :: ISEC_PP_CTRL :: IPV4_CHKSUM_CHK_EN [38:38] */
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_IPV4_CHKSUM_CHK_EN_MASK      BCHP_UINT64_C(0x00000040, 0x00000000)
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_IPV4_CHKSUM_CHK_EN_SHIFT     38
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_IPV4_CHKSUM_CHK_EN_DEFAULT   1

/* ISEC_PORT_0 :: ISEC_PP_CTRL :: SECTAG_AFTER_UDP_HDR_EN [37:37] */
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_SECTAG_AFTER_UDP_HDR_EN_MASK BCHP_UINT64_C(0x00000020, 0x00000000)
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_SECTAG_AFTER_UDP_HDR_EN_SHIFT 37
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_SECTAG_AFTER_UDP_HDR_EN_DEFAULT 1

/* ISEC_PORT_0 :: ISEC_PP_CTRL :: SECTAG_AFTER_TCP_HDR_EN [36:36] */
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_SECTAG_AFTER_TCP_HDR_EN_MASK BCHP_UINT64_C(0x00000010, 0x00000000)
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_SECTAG_AFTER_TCP_HDR_EN_SHIFT 36
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_SECTAG_AFTER_TCP_HDR_EN_DEFAULT 1

/* ISEC_PORT_0 :: ISEC_PP_CTRL :: SECTAG_AFTER_IPV6_HDR_EN [35:35] */
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_SECTAG_AFTER_IPV6_HDR_EN_MASK BCHP_UINT64_C(0x00000008, 0x00000000)
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_SECTAG_AFTER_IPV6_HDR_EN_SHIFT 35
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_SECTAG_AFTER_IPV6_HDR_EN_DEFAULT 1

/* ISEC_PORT_0 :: ISEC_PP_CTRL :: SECTAG_AFTER_IPV4_HDR_EN [34:34] */
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_SECTAG_AFTER_IPV4_HDR_EN_MASK BCHP_UINT64_C(0x00000004, 0x00000000)
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_SECTAG_AFTER_IPV4_HDR_EN_SHIFT 34
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_SECTAG_AFTER_IPV4_HDR_EN_DEFAULT 1

/* ISEC_PORT_0 :: ISEC_PP_CTRL :: PTP_MATCH_RULE_EN [33:30] */
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_PTP_MATCH_RULE_EN_MASK       BCHP_UINT64_C(0x00000003, 0xc0000000)
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_PTP_MATCH_RULE_EN_SHIFT      30
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_PTP_MATCH_RULE_EN_DEFAULT    0

/* ISEC_PORT_0 :: ISEC_PP_CTRL :: PTP_DEST_PORT_EN [29:29] */
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_PTP_DEST_PORT_EN_MASK        BCHP_UINT64_C(0x00000000, 0x20000000)
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_PTP_DEST_PORT_EN_SHIFT       29
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_PTP_DEST_PORT_EN_DEFAULT     0

/* ISEC_PORT_0 :: ISEC_PP_CTRL :: TCP_PROTO_EN [28:28] */
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_TCP_PROTO_EN_MASK            BCHP_UINT64_C(0x00000000, 0x10000000)
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_TCP_PROTO_EN_SHIFT           28
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_TCP_PROTO_EN_DEFAULT         0

/* ISEC_PORT_0 :: ISEC_PP_CTRL :: UDP_PROTO_EN [27:27] */
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_UDP_PROTO_EN_MASK            BCHP_UINT64_C(0x00000000, 0x08000000)
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_UDP_PROTO_EN_SHIFT           27
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_UDP_PROTO_EN_DEFAULT         0

/* ISEC_PORT_0 :: ISEC_PP_CTRL :: PE_ETYPE_EN [26:26] */
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_PE_ETYPE_EN_MASK             BCHP_UINT64_C(0x00000000, 0x04000000)
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_PE_ETYPE_EN_SHIFT            26
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_PE_ETYPE_EN_DEFAULT          0

/* ISEC_PORT_0 :: ISEC_PP_CTRL :: NIV_ETYPE_EN [25:25] */
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_NIV_ETYPE_EN_MASK            BCHP_UINT64_C(0x00000000, 0x02000000)
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_NIV_ETYPE_EN_SHIFT           25
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_NIV_ETYPE_EN_DEFAULT         0

/* ISEC_PORT_0 :: ISEC_PP_CTRL :: PTP_ETYPE_EN [24:24] */
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_PTP_ETYPE_EN_MASK            BCHP_UINT64_C(0x00000000, 0x01000000)
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_PTP_ETYPE_EN_SHIFT           24
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_PTP_ETYPE_EN_DEFAULT         0

/* ISEC_PORT_0 :: ISEC_PP_CTRL :: IPV6_ETYPE_EN [23:23] */
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_IPV6_ETYPE_EN_MASK           BCHP_UINT64_C(0x00000000, 0x00800000)
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_IPV6_ETYPE_EN_SHIFT          23
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_IPV6_ETYPE_EN_DEFAULT        0

/* ISEC_PORT_0 :: ISEC_PP_CTRL :: IPV4_ETYPE_EN [22:22] */
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_IPV4_ETYPE_EN_MASK           BCHP_UINT64_C(0x00000000, 0x00400000)
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_IPV4_ETYPE_EN_SHIFT          22
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_IPV4_ETYPE_EN_DEFAULT        0

/* ISEC_PORT_0 :: ISEC_PP_CTRL :: PBB_EN [21:21] */
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_PBB_EN_MASK                  BCHP_UINT64_C(0x00000000, 0x00200000)
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_PBB_EN_SHIFT                 21
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_PBB_EN_DEFAULT               0

/* ISEC_PORT_0 :: ISEC_PP_CTRL :: MPLS_ETYPE_EN [20:17] */
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_MPLS_ETYPE_EN_MASK           BCHP_UINT64_C(0x00000000, 0x001e0000)
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_MPLS_ETYPE_EN_SHIFT          17
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_MPLS_ETYPE_EN_DEFAULT        0

/* ISEC_PORT_0 :: ISEC_PP_CTRL :: TPID_EN [16:12] */
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_TPID_EN_MASK                 BCHP_UINT64_C(0x00000000, 0x0001f000)
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_TPID_EN_SHIFT                12
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_TPID_EN_DEFAULT              0

/* ISEC_PORT_0 :: ISEC_PP_CTRL :: SECTAG_VLD_RULE_EN [11:03] */
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_SECTAG_VLD_RULE_EN_MASK      BCHP_UINT64_C(0x00000000, 0x00000ff8)
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_SECTAG_VLD_RULE_EN_SHIFT     3
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_SECTAG_VLD_RULE_EN_DEFAULT   0

/* ISEC_PORT_0 :: ISEC_PP_CTRL :: SECTAG_V [02:02] */
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_SECTAG_V_MASK                BCHP_UINT64_C(0x00000000, 0x00000004)
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_SECTAG_V_SHIFT               2
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_SECTAG_V_DEFAULT             0

/* ISEC_PORT_0 :: ISEC_PP_CTRL :: SECTAG_ETYPE_SEL [01:00] */
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_SECTAG_ETYPE_SEL_MASK        BCHP_UINT64_C(0x00000000, 0x00000003)
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_SECTAG_ETYPE_SEL_SHIFT       0
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_SECTAG_ETYPE_SEL_DEFAULT     0

/* Added Defines */
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_WIDTH             64
#define BCHP_ISEC_PORT_0_ISEC_PP_CTRL_NUM_FLDS          22

#define MTU_fld                                           21
#define VXLAN_IPV6_W_UDP_SPTCAM_UDF_PAYLD_SEL_fld         20
#define IPV4_CHKSUM_CHK_EN_fld                            19
#define SECTAG_AFTER_UDP_HDR_EN_fld                       18
#define SECTAG_AFTER_TCP_HDR_EN_fld                       17
#define SECTAG_AFTER_IPV6_HDR_EN_fld                      16
#define SECTAG_AFTER_IPV4_HDR_EN_fld                      15
#define PTP_MATCH_RULE_EN_fld                             14
#define PTP_DEST_PORT_EN_fld                              13
#define TCP_PROTO_EN_fld                                  12
#define UDP_PROTO_EN_fld                                  11
#define PE_ETYPE_EN_fld                                   10
#define NIV_ETYPE_EN_fld                                  9
#define PTP_ETYPE_EN_fld                                  8
#define IPV6_ETYPE_EN_fld                                 7
#define IPV4_ETYPE_EN_fld                                 6
#define PBB_EN_fld                                        5
#define MPLS_ETYPE_EN_fld                                 4
#define TPID_EN_fld                                       3
#define SECTAG_VLD_RULE_EN_fld                            2
#define SECTAG_V_fld                                      1
#define SECTAG_ETYPE_SEL_fld                              0

/***************************************************************************
 *ISEC_RUD_MGMT_RULE_CTRL - Control register for the rules used in Rudimentry Management packet detection Register
 ***************************************************************************/
/* ISEC_PORT_0 :: ISEC_RUD_MGMT_RULE_CTRL :: reserved0 [31:31] */
#define BCHP_ISEC_PORT_0_ISEC_RUD_MGMT_RULE_CTRL_reserved0_MASK    0x80000000u
#define BCHP_ISEC_PORT_0_ISEC_RUD_MGMT_RULE_CTRL_reserved0_SHIFT   31

/* ISEC_PORT_0 :: ISEC_RUD_MGMT_RULE_CTRL :: RULE_SP_NUM [30:23] */
#define BCHP_ISEC_PORT_0_ISEC_RUD_MGMT_RULE_CTRL_RULE_SP_NUM_MASK  0x7f800000u
#define BCHP_ISEC_PORT_0_ISEC_RUD_MGMT_RULE_CTRL_RULE_SP_NUM_SHIFT 23
#define BCHP_ISEC_PORT_0_ISEC_RUD_MGMT_RULE_CTRL_RULE_SP_NUM_DEFAULT 0x00000000

/* ISEC_PORT_0 :: ISEC_RUD_MGMT_RULE_CTRL :: RULE_EN [22:00] */
#define BCHP_ISEC_PORT_0_ISEC_RUD_MGMT_RULE_CTRL_RULE_EN_MASK      0x007fffffu
#define BCHP_ISEC_PORT_0_ISEC_RUD_MGMT_RULE_CTRL_RULE_EN_SHIFT     0
#define BCHP_ISEC_PORT_0_ISEC_RUD_MGMT_RULE_CTRL_RULE_EN_DEFAULT   0x00000000

/* Added Defines */
#define BCHP_ISEC_PORT_0_ISEC_RUD_MGMT_RULE_CTRL_NUM_FLDS          2

#define RULE_EN_fld                                                1
#define RULE_SP_NUM_fld                                            0

/***************************************************************************
 *ISEC_AES_ICV_FAIL_CNT - ICV fail counter: Number of packets for ICV failure. Clear on read. Register
 ***************************************************************************/
/* ISEC_PORT_0 :: ISEC_AES_ICV_FAIL_CNT :: ICV_FAIL_CNT [31:00] */
#define BCHP_ISEC_PORT_0_ISEC_AES_ICV_FAIL_CNT_ICV_FAIL_CNT_MASK   0xffffffffu
#define BCHP_ISEC_PORT_0_ISEC_AES_ICV_FAIL_CNT_ICV_FAIL_CNT_SHIFT  0
#define BCHP_ISEC_PORT_0_ISEC_AES_ICV_FAIL_CNT_ICV_FAIL_CNT_DEFAULT 0x00000000

#define BCHP_ISEC_PORT_0_ISEC_AES_ICV_FAIL_CNT_NUM_FLDS            1
#define ICV_FAIL_CNT_fld                                           0
/***************************************************************************
 *ISEC_MTU_FAIL_CNT - MTU check fail counter: Number of MTU fail packets. Clear on read. Register
 ***************************************************************************/
/* ISEC_PORT_0 :: ISEC_MTU_FAIL_CNT :: MTU_FAIL_CNT [31:00] */
#define BCHP_ISEC_PORT_0_ISEC_MTU_FAIL_CNT_MTU_FAIL_CNT_MASK       0xffffffffu
#define BCHP_ISEC_PORT_0_ISEC_MTU_FAIL_CNT_MTU_FAIL_CNT_SHIFT      0
#define BCHP_ISEC_PORT_0_ISEC_MTU_FAIL_CNT_MTU_FAIL_CNT_DEFAULT    0x00000000

#define BCHP_ISEC_PORT_0_ISEC_MTU_FAIL_CNT_NUM_FLDS                1
#define MTU_FAIL_CNT_fld                                           0
/***************************************************************************
 *ISEC_PDF_STATUS - Ingress parser-data FIFO status Register
 ***************************************************************************/
/* ISEC_PORT_0 :: ISEC_PDF_STATUS :: reserved0 [31:11] */
#define BCHP_ISEC_PORT_0_ISEC_PDF_STATUS_reserved0_MASK            0xfffff800u
#define BCHP_ISEC_PORT_0_ISEC_PDF_STATUS_reserved0_SHIFT           11

/* ISEC_PORT_0 :: ISEC_PDF_STATUS :: CELL_CNT [10:04] */
#define BCHP_ISEC_PORT_0_ISEC_PDF_STATUS_CELL_CNT_MASK             0x000007f0u
#define BCHP_ISEC_PORT_0_ISEC_PDF_STATUS_CELL_CNT_SHIFT            4
#define BCHP_ISEC_PORT_0_ISEC_PDF_STATUS_CELL_CNT_DEFAULT          0x00000000

/* ISEC_PORT_0 :: ISEC_PDF_STATUS :: EMPTY [03:03] */
#define BCHP_ISEC_PORT_0_ISEC_PDF_STATUS_EMPTY_MASK                0x00000008u
#define BCHP_ISEC_PORT_0_ISEC_PDF_STATUS_EMPTY_SHIFT               3
#define BCHP_ISEC_PORT_0_ISEC_PDF_STATUS_EMPTY_DEFAULT             0x00000001

/* ISEC_PORT_0 :: ISEC_PDF_STATUS :: OVERFLOW [02:02] */
#define BCHP_ISEC_PORT_0_ISEC_PDF_STATUS_OVERFLOW_MASK             0x00000004u
#define BCHP_ISEC_PORT_0_ISEC_PDF_STATUS_OVERFLOW_SHIFT            2
#define BCHP_ISEC_PORT_0_ISEC_PDF_STATUS_OVERFLOW_DEFAULT          0x00000000

/* ISEC_PORT_0 :: ISEC_PDF_STATUS :: DOUBLE_BIT_ERR [01:01] */
#define BCHP_ISEC_PORT_0_ISEC_PDF_STATUS_DOUBLE_BIT_ERR_MASK       0x00000002u
#define BCHP_ISEC_PORT_0_ISEC_PDF_STATUS_DOUBLE_BIT_ERR_SHIFT      1
#define BCHP_ISEC_PORT_0_ISEC_PDF_STATUS_DOUBLE_BIT_ERR_DEFAULT    0x00000000

/* ISEC_PORT_0 :: ISEC_PDF_STATUS :: SINGLE_BIT_ERR [00:00] */
#define BCHP_ISEC_PORT_0_ISEC_PDF_STATUS_SINGLE_BIT_ERR_MASK       0x00000001u
#define BCHP_ISEC_PORT_0_ISEC_PDF_STATUS_SINGLE_BIT_ERR_SHIFT      0
#define BCHP_ISEC_PORT_0_ISEC_PDF_STATUS_SINGLE_BIT_ERR_DEFAULT    0x00000000

#define BCHP_ISEC_PORT_0_ISEC_PDF_STATUS_NUM_FLDS                  5
#define ISEC_CELL_CNT_fld                                          4
/***************************************************************************
 *ISEC_PCF_BANK0_STATUS - Ingress parser-control FIFO Bank 0 status Register
 ***************************************************************************/
/* ISEC_PORT_0 :: ISEC_PCF_BANK0_STATUS :: reserved0 [31:11] */
#define BCHP_ISEC_PORT_0_ISEC_PCF_BANK0_STATUS_reserved0_MASK      0xfffff800u
#define BCHP_ISEC_PORT_0_ISEC_PCF_BANK0_STATUS_reserved0_SHIFT     11

/* ISEC_PORT_0 :: ISEC_PCF_BANK0_STATUS :: CELL_CNT [10:04] */
#define BCHP_ISEC_PORT_0_ISEC_PCF_BANK0_STATUS_CELL_CNT_MASK       0x000007f0u
#define BCHP_ISEC_PORT_0_ISEC_PCF_BANK0_STATUS_CELL_CNT_SHIFT      4
#define BCHP_ISEC_PORT_0_ISEC_PCF_BANK0_STATUS_CELL_CNT_DEFAULT    0x00000000

/* ISEC_PORT_0 :: ISEC_PCF_BANK0_STATUS :: EMPTY [03:03] */
#define BCHP_ISEC_PORT_0_ISEC_PCF_BANK0_STATUS_EMPTY_MASK          0x00000008u
#define BCHP_ISEC_PORT_0_ISEC_PCF_BANK0_STATUS_EMPTY_SHIFT         3
#define BCHP_ISEC_PORT_0_ISEC_PCF_BANK0_STATUS_EMPTY_DEFAULT       0x00000001

/* ISEC_PORT_0 :: ISEC_PCF_BANK0_STATUS :: OVERFLOW [02:02] */
#define BCHP_ISEC_PORT_0_ISEC_PCF_BANK0_STATUS_OVERFLOW_MASK       0x00000004u
#define BCHP_ISEC_PORT_0_ISEC_PCF_BANK0_STATUS_OVERFLOW_SHIFT      2
#define BCHP_ISEC_PORT_0_ISEC_PCF_BANK0_STATUS_OVERFLOW_DEFAULT    0x00000000

/* ISEC_PORT_0 :: ISEC_PCF_BANK0_STATUS :: DOUBLE_BIT_ERR [01:01] */
#define BCHP_ISEC_PORT_0_ISEC_PCF_BANK0_STATUS_DOUBLE_BIT_ERR_MASK 0x00000002u
#define BCHP_ISEC_PORT_0_ISEC_PCF_BANK0_STATUS_DOUBLE_BIT_ERR_SHIFT 1
#define BCHP_ISEC_PORT_0_ISEC_PCF_BANK0_STATUS_DOUBLE_BIT_ERR_DEFAULT 0x00000000

/* ISEC_PORT_0 :: ISEC_PCF_BANK0_STATUS :: SINGLE_BIT_ERR [00:00] */
#define BCHP_ISEC_PORT_0_ISEC_PCF_BANK0_STATUS_SINGLE_BIT_ERR_MASK 0x00000001u
#define BCHP_ISEC_PORT_0_ISEC_PCF_BANK0_STATUS_SINGLE_BIT_ERR_SHIFT 0
#define BCHP_ISEC_PORT_0_ISEC_PCF_BANK0_STATUS_SINGLE_BIT_ERR_DEFAULT 0x00000000

#define BCHP_ISEC_PORT_0_ISEC_PCF_BANK0_STATUS_NUM_FLDS            5
/***************************************************************************
 *ISEC_PCF_BANK1_STATUS - Ingress parser-control FIFO Bank 1 status Register
 ***************************************************************************/
/* ISEC_PORT_0 :: ISEC_PCF_BANK1_STATUS :: reserved0 [31:11] */
#define BCHP_ISEC_PORT_0_ISEC_PCF_BANK1_STATUS_reserved0_MASK      0xfffff800u
#define BCHP_ISEC_PORT_0_ISEC_PCF_BANK1_STATUS_reserved0_SHIFT     11

/* ISEC_PORT_0 :: ISEC_PCF_BANK1_STATUS :: CELL_CNT [10:04] */
#define BCHP_ISEC_PORT_0_ISEC_PCF_BANK1_STATUS_CELL_CNT_MASK       0x000007f0u
#define BCHP_ISEC_PORT_0_ISEC_PCF_BANK1_STATUS_CELL_CNT_SHIFT      4
#define BCHP_ISEC_PORT_0_ISEC_PCF_BANK1_STATUS_CELL_CNT_DEFAULT    0x00000000

/* ISEC_PORT_0 :: ISEC_PCF_BANK1_STATUS :: EMPTY [03:03] */
#define BCHP_ISEC_PORT_0_ISEC_PCF_BANK1_STATUS_EMPTY_MASK          0x00000008u
#define BCHP_ISEC_PORT_0_ISEC_PCF_BANK1_STATUS_EMPTY_SHIFT         3
#define BCHP_ISEC_PORT_0_ISEC_PCF_BANK1_STATUS_EMPTY_DEFAULT       0x00000001

/* ISEC_PORT_0 :: ISEC_PCF_BANK1_STATUS :: OVERFLOW [02:02] */
#define BCHP_ISEC_PORT_0_ISEC_PCF_BANK1_STATUS_OVERFLOW_MASK       0x00000004u
#define BCHP_ISEC_PORT_0_ISEC_PCF_BANK1_STATUS_OVERFLOW_SHIFT      2
#define BCHP_ISEC_PORT_0_ISEC_PCF_BANK1_STATUS_OVERFLOW_DEFAULT    0x00000000

/* ISEC_PORT_0 :: ISEC_PCF_BANK1_STATUS :: DOUBLE_BIT_ERR [01:01] */
#define BCHP_ISEC_PORT_0_ISEC_PCF_BANK1_STATUS_DOUBLE_BIT_ERR_MASK 0x00000002u
#define BCHP_ISEC_PORT_0_ISEC_PCF_BANK1_STATUS_DOUBLE_BIT_ERR_SHIFT 1
#define BCHP_ISEC_PORT_0_ISEC_PCF_BANK1_STATUS_DOUBLE_BIT_ERR_DEFAULT 0x00000000

/* ISEC_PORT_0 :: ISEC_PCF_BANK1_STATUS :: SINGLE_BIT_ERR [00:00] */
#define BCHP_ISEC_PORT_0_ISEC_PCF_BANK1_STATUS_SINGLE_BIT_ERR_MASK 0x00000001u
#define BCHP_ISEC_PORT_0_ISEC_PCF_BANK1_STATUS_SINGLE_BIT_ERR_SHIFT 0
#define BCHP_ISEC_PORT_0_ISEC_PCF_BANK1_STATUS_SINGLE_BIT_ERR_DEFAULT 0x00000000

#define BCHP_ISEC_PORT_0_ISEC_PCF_BANK1_STATUS_NUM_FLDS            5
/***************************************************************************
 *ISEC_CB_STR_FIFO_STATUS - Ingress internal Control Bus Storage FIFO status Register
 ***************************************************************************/
/* ISEC_PORT_0 :: ISEC_CB_STR_FIFO_STATUS :: reserved0 [31:02] */
#define BCHP_ISEC_PORT_0_ISEC_CB_STR_FIFO_STATUS_reserved0_MASK    0xfffffffcu
#define BCHP_ISEC_PORT_0_ISEC_CB_STR_FIFO_STATUS_reserved0_SHIFT   2

/* ISEC_PORT_0 :: ISEC_CB_STR_FIFO_STATUS :: DOUBLE_BIT_ERR [01:01] */
#define BCHP_ISEC_PORT_0_ISEC_CB_STR_FIFO_STATUS_DOUBLE_BIT_ERR_MASK 0x00000002u
#define BCHP_ISEC_PORT_0_ISEC_CB_STR_FIFO_STATUS_DOUBLE_BIT_ERR_SHIFT 1
#define BCHP_ISEC_PORT_0_ISEC_CB_STR_FIFO_STATUS_DOUBLE_BIT_ERR_DEFAULT 0x00000000

/* ISEC_PORT_0 :: ISEC_CB_STR_FIFO_STATUS :: SINGLE_BIT_ERR [00:00] */
#define BCHP_ISEC_PORT_0_ISEC_CB_STR_FIFO_STATUS_SINGLE_BIT_ERR_MASK 0x00000001u
#define BCHP_ISEC_PORT_0_ISEC_CB_STR_FIFO_STATUS_SINGLE_BIT_ERR_SHIFT 0
#define BCHP_ISEC_PORT_0_ISEC_CB_STR_FIFO_STATUS_SINGLE_BIT_ERR_DEFAULT 0x00000000

#define BCHP_ISEC_PORT_0_ISEC_CB_STR_FIFO_STATUS_NUM_FLDS          2
/***************************************************************************
 *ISEC_IDF_STATUS - Ingress input-data FIFO status Register
 ***************************************************************************/
/* ISEC_PORT_0 :: ISEC_IDF_STATUS :: reserved0 [31:13] */
#define BCHP_ISEC_PORT_0_ISEC_IDF_STATUS_reserved0_MASK            0xffffe000u
#define BCHP_ISEC_PORT_0_ISEC_IDF_STATUS_reserved0_SHIFT           13

/* ISEC_PORT_0 :: ISEC_IDF_STATUS :: CELL_CNT [12:04] */
#define BCHP_ISEC_PORT_0_ISEC_IDF_STATUS_CELL_CNT_MASK             0x00001ff0u
#define BCHP_ISEC_PORT_0_ISEC_IDF_STATUS_CELL_CNT_SHIFT            4
#define BCHP_ISEC_PORT_0_ISEC_IDF_STATUS_CELL_CNT_DEFAULT          0x00000000

/* ISEC_PORT_0 :: ISEC_IDF_STATUS :: EMPTY [03:03] */
#define BCHP_ISEC_PORT_0_ISEC_IDF_STATUS_EMPTY_MASK                0x00000008u
#define BCHP_ISEC_PORT_0_ISEC_IDF_STATUS_EMPTY_SHIFT               3
#define BCHP_ISEC_PORT_0_ISEC_IDF_STATUS_EMPTY_DEFAULT             0x00000001

/* ISEC_PORT_0 :: ISEC_IDF_STATUS :: OVERFLOW [02:02] */
#define BCHP_ISEC_PORT_0_ISEC_IDF_STATUS_OVERFLOW_MASK             0x00000004u
#define BCHP_ISEC_PORT_0_ISEC_IDF_STATUS_OVERFLOW_SHIFT            2
#define BCHP_ISEC_PORT_0_ISEC_IDF_STATUS_OVERFLOW_DEFAULT          0x00000000

/* ISEC_PORT_0 :: ISEC_IDF_STATUS :: DOUBLE_BIT_ERR [01:01] */
#define BCHP_ISEC_PORT_0_ISEC_IDF_STATUS_DOUBLE_BIT_ERR_MASK       0x00000002u
#define BCHP_ISEC_PORT_0_ISEC_IDF_STATUS_DOUBLE_BIT_ERR_SHIFT      1
#define BCHP_ISEC_PORT_0_ISEC_IDF_STATUS_DOUBLE_BIT_ERR_DEFAULT    0x00000000

/* ISEC_PORT_0 :: ISEC_IDF_STATUS :: SINGLE_BIT_ERR [00:00] */
#define BCHP_ISEC_PORT_0_ISEC_IDF_STATUS_SINGLE_BIT_ERR_MASK       0x00000001u
#define BCHP_ISEC_PORT_0_ISEC_IDF_STATUS_SINGLE_BIT_ERR_SHIFT      0
#define BCHP_ISEC_PORT_0_ISEC_IDF_STATUS_SINGLE_BIT_ERR_DEFAULT    0x00000000

#define BCHP_ISEC_PORT_0_ISEC_IDF_STATUS_NUM_FLDS                  5

/***************************************************************************
 *ISEC_ICF_STATUS - Ingress input-control FIFO status Register
 ***************************************************************************/
/* ISEC_PORT_0 :: ISEC_ICF_STATUS :: reserved0 [31:13] */
#define BCHP_ISEC_PORT_0_ISEC_ICF_STATUS_reserved0_MASK            0xffffe000u
#define BCHP_ISEC_PORT_0_ISEC_ICF_STATUS_reserved0_SHIFT           13

/* ISEC_PORT_0 :: ISEC_ICF_STATUS :: CELL_CNT [12:04] */
#define BCHP_ISEC_PORT_0_ISEC_ICF_STATUS_CELL_CNT_MASK             0x00001ff0u
#define BCHP_ISEC_PORT_0_ISEC_ICF_STATUS_CELL_CNT_SHIFT            4
#define BCHP_ISEC_PORT_0_ISEC_ICF_STATUS_CELL_CNT_DEFAULT          0x00000000

/* ISEC_PORT_0 :: ISEC_ICF_STATUS :: EMPTY [03:03] */
#define BCHP_ISEC_PORT_0_ISEC_ICF_STATUS_EMPTY_MASK                0x00000008u
#define BCHP_ISEC_PORT_0_ISEC_ICF_STATUS_EMPTY_SHIFT               3
#define BCHP_ISEC_PORT_0_ISEC_ICF_STATUS_EMPTY_DEFAULT             0x00000001

/* ISEC_PORT_0 :: ISEC_ICF_STATUS :: OVERFLOW [02:02] */
#define BCHP_ISEC_PORT_0_ISEC_ICF_STATUS_OVERFLOW_MASK             0x00000004u
#define BCHP_ISEC_PORT_0_ISEC_ICF_STATUS_OVERFLOW_SHIFT            2
#define BCHP_ISEC_PORT_0_ISEC_ICF_STATUS_OVERFLOW_DEFAULT          0x00000000

/* ISEC_PORT_0 :: ISEC_ICF_STATUS :: DOUBLE_BIT_ERR [01:01] */
#define BCHP_ISEC_PORT_0_ISEC_ICF_STATUS_DOUBLE_BIT_ERR_MASK       0x00000002u
#define BCHP_ISEC_PORT_0_ISEC_ICF_STATUS_DOUBLE_BIT_ERR_SHIFT      1
#define BCHP_ISEC_PORT_0_ISEC_ICF_STATUS_DOUBLE_BIT_ERR_DEFAULT    0x00000000

/* ISEC_PORT_0 :: ISEC_ICF_STATUS :: SINGLE_BIT_ERR [00:00] */
#define BCHP_ISEC_PORT_0_ISEC_ICF_STATUS_SINGLE_BIT_ERR_MASK       0x00000001u
#define BCHP_ISEC_PORT_0_ISEC_ICF_STATUS_SINGLE_BIT_ERR_SHIFT      0
#define BCHP_ISEC_PORT_0_ISEC_ICF_STATUS_SINGLE_BIT_ERR_DEFAULT    0x00000000

#define BCHP_ISEC_PORT_0_ISEC_ICF_STATUS_NUM_FLDS                  5
/***************************************************************************
 *ISEC_ODF_STATUS - Ingress output-data FIFO status Register
 ***************************************************************************/
/* ISEC_PORT_0 :: ISEC_ODF_STATUS :: reserved0 [31:11] */
#define BCHP_ISEC_PORT_0_ISEC_ODF_STATUS_reserved0_MASK            0xfffff800u
#define BCHP_ISEC_PORT_0_ISEC_ODF_STATUS_reserved0_SHIFT           11

/* ISEC_PORT_0 :: ISEC_ODF_STATUS :: CELL_CNT [10:04] */
#define BCHP_ISEC_PORT_0_ISEC_ODF_STATUS_CELL_CNT_MASK             0x000007f0u
#define BCHP_ISEC_PORT_0_ISEC_ODF_STATUS_CELL_CNT_SHIFT            4
#define BCHP_ISEC_PORT_0_ISEC_ODF_STATUS_CELL_CNT_DEFAULT          0x00000000

/* ISEC_PORT_0 :: ISEC_ODF_STATUS :: EMPTY [03:03] */
#define BCHP_ISEC_PORT_0_ISEC_ODF_STATUS_EMPTY_MASK                0x00000008u
#define BCHP_ISEC_PORT_0_ISEC_ODF_STATUS_EMPTY_SHIFT               3
#define BCHP_ISEC_PORT_0_ISEC_ODF_STATUS_EMPTY_DEFAULT             0x00000001

/* ISEC_PORT_0 :: ISEC_ODF_STATUS :: OVERFLOW [02:02] */
#define BCHP_ISEC_PORT_0_ISEC_ODF_STATUS_OVERFLOW_MASK             0x00000004u
#define BCHP_ISEC_PORT_0_ISEC_ODF_STATUS_OVERFLOW_SHIFT            2
#define BCHP_ISEC_PORT_0_ISEC_ODF_STATUS_OVERFLOW_DEFAULT          0x00000000

/* ISEC_PORT_0 :: ISEC_ODF_STATUS :: DOUBLE_BIT_ERR [01:01] */
#define BCHP_ISEC_PORT_0_ISEC_ODF_STATUS_DOUBLE_BIT_ERR_MASK       0x00000002u
#define BCHP_ISEC_PORT_0_ISEC_ODF_STATUS_DOUBLE_BIT_ERR_SHIFT      1
#define BCHP_ISEC_PORT_0_ISEC_ODF_STATUS_DOUBLE_BIT_ERR_DEFAULT    0x00000000

/* ISEC_PORT_0 :: ISEC_ODF_STATUS :: SINGLE_BIT_ERR [00:00] */
#define BCHP_ISEC_PORT_0_ISEC_ODF_STATUS_SINGLE_BIT_ERR_MASK       0x00000001u
#define BCHP_ISEC_PORT_0_ISEC_ODF_STATUS_SINGLE_BIT_ERR_SHIFT      0
#define BCHP_ISEC_PORT_0_ISEC_ODF_STATUS_SINGLE_BIT_ERR_DEFAULT    0x00000000

#define BCHP_ISEC_PORT_0_ISEC_ODF_STATUS_NUM_FLDS                  5
/***************************************************************************
 *ISEC_TAG_FIFO_STATUS - Ingress tag-FIFO status Register
 ***************************************************************************/
/* ISEC_PORT_0 :: ISEC_TAG_FIFO_STATUS :: reserved0 [31:04] */
#define BCHP_ISEC_PORT_0_ISEC_TAG_FIFO_STATUS_reserved0_MASK       0xfffffff0u
#define BCHP_ISEC_PORT_0_ISEC_TAG_FIFO_STATUS_reserved0_SHIFT      4

/* ISEC_PORT_0 :: ISEC_TAG_FIFO_STATUS :: EMPTY [03:03] */
#define BCHP_ISEC_PORT_0_ISEC_TAG_FIFO_STATUS_EMPTY_MASK           0x00000008u
#define BCHP_ISEC_PORT_0_ISEC_TAG_FIFO_STATUS_EMPTY_SHIFT          3
#define BCHP_ISEC_PORT_0_ISEC_TAG_FIFO_STATUS_EMPTY_DEFAULT        0x00000001

/* ISEC_PORT_0 :: ISEC_TAG_FIFO_STATUS :: OVERFLOW [02:02] */
#define BCHP_ISEC_PORT_0_ISEC_TAG_FIFO_STATUS_OVERFLOW_MASK        0x00000004u
#define BCHP_ISEC_PORT_0_ISEC_TAG_FIFO_STATUS_OVERFLOW_SHIFT       2
#define BCHP_ISEC_PORT_0_ISEC_TAG_FIFO_STATUS_OVERFLOW_DEFAULT     0x00000000

/* ISEC_PORT_0 :: ISEC_TAG_FIFO_STATUS :: DOUBLE_BIT_ERR [01:01] */
#define BCHP_ISEC_PORT_0_ISEC_TAG_FIFO_STATUS_DOUBLE_BIT_ERR_MASK  0x00000002u
#define BCHP_ISEC_PORT_0_ISEC_TAG_FIFO_STATUS_DOUBLE_BIT_ERR_SHIFT 1
#define BCHP_ISEC_PORT_0_ISEC_TAG_FIFO_STATUS_DOUBLE_BIT_ERR_DEFAULT 0x00000000

/* ISEC_PORT_0 :: ISEC_TAG_FIFO_STATUS :: SINGLE_BIT_ERR [00:00] */
#define BCHP_ISEC_PORT_0_ISEC_TAG_FIFO_STATUS_SINGLE_BIT_ERR_MASK  0x00000001u
#define BCHP_ISEC_PORT_0_ISEC_TAG_FIFO_STATUS_SINGLE_BIT_ERR_SHIFT 0
#define BCHP_ISEC_PORT_0_ISEC_TAG_FIFO_STATUS_SINGLE_BIT_ERR_DEFAULT 0x00000000

#define BCHP_ISEC_PORT_0_ISEC_TAG_FIFO_STATUS_NUM_FLDS             4
/***************************************************************************
 *ISEC_CW_FIFO_STATUS - Ingress CW FIFO status Register
 ***************************************************************************/
/* ISEC_PORT_0 :: ISEC_CW_FIFO_STATUS :: reserved0 [31:13] */
#define BCHP_ISEC_PORT_0_ISEC_CW_FIFO_STATUS_reserved0_MASK        0xffffe000u
#define BCHP_ISEC_PORT_0_ISEC_CW_FIFO_STATUS_reserved0_SHIFT       13

/* ISEC_PORT_0 :: ISEC_CW_FIFO_STATUS :: CREDIT_RESIDUE [12:12] */
#define BCHP_ISEC_PORT_0_ISEC_CW_FIFO_STATUS_CREDIT_RESIDUE_MASK   0x00001000u
#define BCHP_ISEC_PORT_0_ISEC_CW_FIFO_STATUS_CREDIT_RESIDUE_SHIFT  12
#define BCHP_ISEC_PORT_0_ISEC_CW_FIFO_STATUS_CREDIT_RESIDUE_DEFAULT 0x00000000

/* ISEC_PORT_0 :: ISEC_CW_FIFO_STATUS :: CELL_CNT [11:08] */
#define BCHP_ISEC_PORT_0_ISEC_CW_FIFO_STATUS_CELL_CNT_MASK         0x00000f00u
#define BCHP_ISEC_PORT_0_ISEC_CW_FIFO_STATUS_CELL_CNT_SHIFT        8
#define BCHP_ISEC_PORT_0_ISEC_CW_FIFO_STATUS_CELL_CNT_DEFAULT      0x00000000

/* ISEC_PORT_0 :: ISEC_CW_FIFO_STATUS :: CELL_REQ_CNT [07:04] */
#define BCHP_ISEC_PORT_0_ISEC_CW_FIFO_STATUS_CELL_REQ_CNT_MASK     0x000000f0u
#define BCHP_ISEC_PORT_0_ISEC_CW_FIFO_STATUS_CELL_REQ_CNT_SHIFT    4
#define BCHP_ISEC_PORT_0_ISEC_CW_FIFO_STATUS_CELL_REQ_CNT_DEFAULT  0x00000000

/* ISEC_PORT_0 :: ISEC_CW_FIFO_STATUS :: EMPTY [03:03] */
#define BCHP_ISEC_PORT_0_ISEC_CW_FIFO_STATUS_EMPTY_MASK            0x00000008u
#define BCHP_ISEC_PORT_0_ISEC_CW_FIFO_STATUS_EMPTY_SHIFT           3
#define BCHP_ISEC_PORT_0_ISEC_CW_FIFO_STATUS_EMPTY_DEFAULT         0x00000001

/* ISEC_PORT_0 :: ISEC_CW_FIFO_STATUS :: OVERFLOW [02:02] */
#define BCHP_ISEC_PORT_0_ISEC_CW_FIFO_STATUS_OVERFLOW_MASK         0x00000004u
#define BCHP_ISEC_PORT_0_ISEC_CW_FIFO_STATUS_OVERFLOW_SHIFT        2
#define BCHP_ISEC_PORT_0_ISEC_CW_FIFO_STATUS_OVERFLOW_DEFAULT      0x00000000

/* ISEC_PORT_0 :: ISEC_CW_FIFO_STATUS :: DOUBLE_BIT_ERR [01:01] */
#define BCHP_ISEC_PORT_0_ISEC_CW_FIFO_STATUS_DOUBLE_BIT_ERR_MASK   0x00000002u
#define BCHP_ISEC_PORT_0_ISEC_CW_FIFO_STATUS_DOUBLE_BIT_ERR_SHIFT  1
#define BCHP_ISEC_PORT_0_ISEC_CW_FIFO_STATUS_DOUBLE_BIT_ERR_DEFAULT 0x00000000

/* ISEC_PORT_0 :: ISEC_CW_FIFO_STATUS :: SINGLE_BIT_ERR [00:00] */
#define BCHP_ISEC_PORT_0_ISEC_CW_FIFO_STATUS_SINGLE_BIT_ERR_MASK   0x00000001u
#define BCHP_ISEC_PORT_0_ISEC_CW_FIFO_STATUS_SINGLE_BIT_ERR_SHIFT  0
#define BCHP_ISEC_PORT_0_ISEC_CW_FIFO_STATUS_SINGLE_BIT_ERR_DEFAULT 0x00000000

#define BCHP_ISEC_PORT_0_ISEC_CW_FIFO_STATUS_NUM_FLDS              7
#endif /* #ifndef BCHP_ISEC_PORT_0_H__ */

/* End of File */
