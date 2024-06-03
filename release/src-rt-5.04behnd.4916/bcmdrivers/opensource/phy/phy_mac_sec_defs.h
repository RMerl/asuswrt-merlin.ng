/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard

    Unless you and Broadcom execute a separate written software license
    agreement governing use of this software, this software is licensed
    to you under the terms of the GNU General Public License version 2
    (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
    with the following added to such license:

       As a special exception, the copyright holders of this software give
       you permission to link this software with independent modules, and
       to copy and distribute the resulting executable under terms of your
       choice, provided that you also meet, for each linked independent
       module, the terms and conditions of the license of that module.
       An independent module is a module which is not derived from this
       software.  The special exception does not apply to any modifications
       of the software.

    Not withstanding the above, under no circumstances may you combine
    this software in any way with any other Broadcom software provided
    under a license other than the GPL, without Broadcom's express prior
    written consent.

:>
*/


#ifndef PHY_MAC_SEC_DEFS_H_
#define PHY_MAC_SEC_DEFS_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

/* Default configuration */
//#include "c_eip160.h"

/* Driver Framework Basic Definitions API */
//#include "basic_defs.h"         /* BIT definitions, uint8, uint32 */


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

#define VPORT_NUM_MAX   16
#define SA_NUM_MAX      32
#define RULE_NUM_MAX    32

#define CRG_CORE_CONFIGr                       0x4187
#define CRG_CORE_CONFIG_LONGFIN_A0_INIT        0x0800
#define CRG_CORE_CONFIG_LONGFIN_A0_INIT_MASK   0x1E00
#define CFG_MACSEC_CTRLr                       0x813C
#define CFG_MACSEC_CTRL__INIT                  0x0010

#define PHY_REG_MACSEC_CTRL_THROUGH        0x0
#define PHY_REG_MACSEC_CTRL_BYPASS         (1U << 4)
#define PHY_REG_MACSEC_CTRL                0x813C
#define PHY_REG_FW_NOTIFY                  0x8118
#define PHY_REG_FW_NOTIFY_MACSEC           (1U << 0)

#define STRAP_CHANGE_TIMEOUT      2     /**< in seconds */

/***************************************/
/* speed definition                  */
/***************************************/
#define SPEED_10                        0 
#define SPEED_100                       1        
#define SPEED_1000                      2
#define SPEED_10G                       3 
#define SPEED_2500                      4        
#define SPEED_5000                      5 
#define SPEED_UNDEFINED                 6       /**< Error case */

#define XFIM_XFI_CTRL0_SPEED_10M                  0x0
#define XFIM_XFI_CTRL0_SPEED_100M                 0x1
#define XFIM_XFI_CTRL0_SPEED_1G                   0x02
#define XFIM_XFI_CTRL0_SPEED_2P5G                 0x04
#define XFIM_XFI_CTRL0_SPEED_5G                   0x08 
#define XFIM_XFI_CTRL0_SPEED_10G                  0x10  

/** Use 64-bit sequence number. */
#define SAB_MACSEC_FLAG_LONGSEQ 0x1
/** Retain SecTAG (debugging).*/
#define SAB_MACSEC_FLAG_RETAIN_SECTAG 0x2
/** Retain ICV (debugging). */
#define SAB_MACSEC_FLAG_RETAIN_ICV 0x4
/** Allow sequence number rollover (debugging).*/
#define SAB_MACSEC_FLAG_ROLLOVER 0x8
/** Enable SA auto update. */
#define SAB_MACSEC_FLAG_UPDATE_ENABLE 0x10
/** Generate IRQ when SA is expired. */
#define SAB_MACSEC_FLAG_SA_EXPIRED_IRQ 0x20
/** Update time stamps (only if hardware supports this feature).*/
#define SAB_MACSEC_FLAG_UPDATE_TIME   0x40

/* using postfix "U" to be compatible with uint32 */
/* ("UL" is not needed and gives lint warning) */
#define BIT_0   0x00000001U
#define BIT_1   0x00000002U
#define BIT_2   0x00000004U
#define BIT_3   0x00000008U
#define BIT_4   0x00000010U
#define BIT_5   0x00000020U
#define BIT_6   0x00000040U
#define BIT_7   0x00000080U
#define BIT_8   0x00000100U
#define BIT_9   0x00000200U
#define BIT_10  0x00000400U
#define BIT_11  0x00000800U
#define BIT_12  0x00001000U
#define BIT_13  0x00002000U
#define BIT_14  0x00004000U
#define BIT_15  0x00008000U
#define BIT_16  0x00010000U
#define BIT_17  0x00020000U
#define BIT_18  0x00040000U
#define BIT_19  0x00080000U
#define BIT_20  0x00100000U
#define BIT_21  0x00200000U
#define BIT_22  0x00400000U
#define BIT_23  0x00800000U
#define BIT_24  0x01000000U
#define BIT_25  0x02000000U
#define BIT_26  0x04000000U
#define BIT_27  0x08000000U
#define BIT_28  0x10000000U
#define BIT_29  0x20000000U
#define BIT_30  0x40000000U
#define BIT_31  0x80000000U


/* ============ MASK_n_BITS ============ */

#define MASK_1_BIT      (BIT_1 - 1)
#define MASK_2_BITS     (BIT_2 - 1)
#define MASK_3_BITS     (BIT_3 - 1)
#define MASK_4_BITS     (BIT_4 - 1)
#define MASK_5_BITS     (BIT_5 - 1)
#define MASK_6_BITS     (BIT_6 - 1)
#define MASK_7_BITS     (BIT_7 - 1)
#define MASK_8_BITS     (BIT_8 - 1)
#define MASK_9_BITS     (BIT_9 - 1)
#define MASK_10_BITS    (BIT_10 - 1)
#define MASK_11_BITS    (BIT_11 - 1)
#define MASK_12_BITS    (BIT_12 - 1)
#define MASK_13_BITS    (BIT_13 - 1)
#define MASK_14_BITS    (BIT_14 - 1)
#define MASK_15_BITS    (BIT_15 - 1)
#define MASK_16_BITS    (BIT_16 - 1)
#define MASK_17_BITS    (BIT_17 - 1)
#define MASK_18_BITS    (BIT_18 - 1)
#define MASK_19_BITS    (BIT_19 - 1)
#define MASK_20_BITS    (BIT_20 - 1)
#define MASK_21_BITS    (BIT_21 - 1)
#define MASK_22_BITS    (BIT_22 - 1)
#define MASK_23_BITS    (BIT_23 - 1)
#define MASK_24_BITS    (BIT_24 - 1)
#define MASK_25_BITS    (BIT_25 - 1)
#define MASK_26_BITS    (BIT_26 - 1)
#define MASK_27_BITS    (BIT_27 - 1)
#define MASK_28_BITS    (BIT_28 - 1)
#define MASK_29_BITS    (BIT_29 - 1)
#define MASK_30_BITS    (BIT_30 - 1)
#define MASK_31_BITS    (BIT_31 - 1)

#define CMD_PASS                           0x0004
#define CMD_ERROR                          0x0008
#define CMD_IN_PROGRESS                    0x0002
#define CMD_SYSTEM_BUSY                    0xBBBB


#define IDENTIFIER_NOT_USED(_v) if(_v){}
#define MACSEC_INIT_RETRY_COUNT 100
#define LIST_DUMMY_LIST_ID          0
#define LIST_INTERNAL_DATA_SIZE     2

/* Object types for free lists. */
#define OBJECT_SA_T 0
#define OBJECT_SC_T 1
#define OBJECT_VPORT_T 2
#define OBJECT_RULE_T 3

#define MACSEC_XLMAC_REG_BASE 0xf100
#define MACSEC_INGRESS_REG_BASE 0xf102
#define MACSEC_EGRESS_REG_BASE 0xf103

#define EIP160_CONF_BASE    0x00
#define EIP217_CONF_BASE    0x00
#define EIP62_CONF_BASE     0xF400

#define EIP217_REG_OPTIONS2                 (EIP217_CONF_BASE + 0xF4)
#define EIP217_REG_OPTIONS                  (EIP217_CONF_BASE + 0xF8)
#define EIP217_REG_VERSION                  (EIP217_CONF_BASE + 0xFC)
#define EIP217_REG_COUNT_CONTROL            (EIP217_CONF_BASE + 0x10)
#define EIP217_REG_OFFS                     4
/* EIP-217 register bank: 64-bit statistics counters (n) */
#define EIP217_REG_COUNTER_OFFS             8
#define EIP217_REG_COUNTER_LO(n)            (EIP217_CONF_BASE + \
                                             EIP217_REG_COUNTER_OFFS * n)
#define EIP217_REG_COUNTER_HI(n)            (EIP217_CONF_BASE + \
                                             EIP217_REG_OFFS + \
                                             EIP217_REG_COUNTER_OFFS * n)

#define EIP62_REG_SEQ_NR_THRESH            (EIP62_CONF_BASE + 0x0020)
#define EIP62_REG_SEQ_NR_THRESH_64_LO      (EIP62_CONF_BASE + 0x0024)
#define EIP62_REG_SEQ_NR_THRESH_64_HI      (EIP62_CONF_BASE + 0x0028)

#define EIP160_REG_GHP_OFFS                 (EIP160_CONF_BASE + 0x7900)
#define EIP160_REG_CONFIG2                  (EIP160_CONF_BASE + 0xFFF4)
#define EIP160_REG_CONFIG                   (EIP160_CONF_BASE + 0xFFF8)
#define EIP160_REG_VERSION                  (EIP160_CONF_BASE + 0xFFFC)
#define EIP160_REG_SAF_CTRL                 (EIP160_CONF_BASE + 0xFE00)
#define EIP160_REG_SAF_THRESHOLD            (EIP160_CONF_BASE + 0xFE04)
#define EIP160_REG_SAM_NM_FLOW_NCP          (EIP160_REG_GHP_OFFS + 0x44)
#define EIP160_REG_SAM_NM_FLOW_CP           (EIP160_REG_GHP_OFFS + 0x48)
#define EIP160_REG_TCAM_STAT_CTRL_OFFS      (EIP160_CONF_BASE + 0x4400)
#define EIP160_REG_SA_STATISTICS_CONTROLS   (EIP160_CONF_BASE + 0xE200)
#define EIP160_REG_IFC_STATISTICS_CONTROLS  (EIP160_CONF_BASE + 0xE600)
#define EIP160_REG_RXCAM_STATISTICS_CONTROLS (EIP160_CONF_BASE + 0x5400)
#define EIP160_REG_SECY_STATISTICS_CONTROLS (EIP160_CONF_BASE + 0xE400)
#define EIP160_REG_FORCE_CLOCK_ON           (EIP160_CONF_BASE + 0xFFEC)
#define EIP160_REG_FORCE_CLOCK_OFF          (EIP160_CONF_BASE + 0xFFF0)

#define EIP160_REG_CPD_OFFS                 (EIP160_CONF_BASE + 0x7800)
#define EIP160_REG_CP_MATCH_ENABLE          (EIP160_REG_CPD_OFFS + 0xFC)

#define EIP160_REG_CP_MATCH_ENABLE_MASK     (BIT_31 | MASK_21_BITS)

#define EIP160_REG_XTSECY_STATISTICS_SET_OFFS         0x80
#define EIP160_REG_XTSECY_STATISTICS_BANK_OFFS        0x10000
#define EIP160_REG_XTSECY_STATISTICS_OFFS             0xA000
#define EIP160_REG_XTSECY_STATISTICS(n)  (EIP160_CONF_BASE + \
                                    EIP160_REG_XTSECY_STATISTICS_OFFS + \
                                    (EIP160_REG_XTSECY_STATISTICS_BANK_OFFS * \
                                        (n >> 5)) + \
                                    (EIP160_REG_XTSECY_STATISTICS_SET_OFFS * \
                                        (n & 0x1F)))

/* Interface statistics counters: n - counter ID */
#define EIP160_REG_IFC_STATISTICS_ID_OFFS           0x80
#define EIP160_REG_IFC_STATISTICS_BANK_OFFS         0x10000
#define EIP160_REG_IFC_STATISTICS_OFFS          0xC000
#define EIP160_REG_IFC_STATISTICS(n)    (EIP160_CONF_BASE + \
                                        EIP160_REG_IFC_STATISTICS_OFFS + \
                                        (EIP160_REG_IFC_STATISTICS_BANK_OFFS* \
                                        (n >> 5)) + \
                                        (EIP160_REG_IFC_STATISTICS_ID_OFFS * \
                                        (n & 0x1F)))

/* SA statistics counters: n - counter ID */
#define EIP160_REG_SA_STATISTICS_ID_OFFS            0x80
#define EIP160_REG_SA_STATISTICS_BANK_OFFS          0x10000
#define EIP160_REG_SA_STATISTICS_OFFS               0x8000
#define EIP160_REG_SA_STATISTICS(n)                 (EIP160_CONF_BASE + \
                                        EIP160_REG_SA_STATISTICS_OFFS + \
                                        (EIP160_REG_SA_STATISTICS_BANK_OFFS * \
                                        (n >> 6)) + \
                                        (EIP160_REG_SA_STATISTICS_ID_OFFS * \
                                        (n & 0x3F)))

/* Flow control: vPort, SecY, SC policy */
#define EIP160_REG_SAM_FLOW_CTRL_OFFS       8
#define EIP160_REG_SAM_FLOW_CTRL1(n)        (EIP160_CONF_BASE + 0x7000 + \
                                             (0x10000 * (n >> 5)) + \
                                             (EIP160_REG_SAM_FLOW_CTRL_OFFS * \
                                              (n & 0x1f)))

#define EIP160_REG_SAM_FLOW_CTRL2(n)        (EIP160_CONF_BASE + 0x7004 + \
                                             (0x10000 * (n >> 5)) + \
                                             (EIP160_REG_SAM_FLOW_CTRL_OFFS * \
                                              (n & 0x1f)))

#define EIP160_REG_SAM_IN_FLIGHT            (EIP160_CONF_BASE + 0x6104)

/* Transform records (SA = Security Association) area */
#define EIP160_XFORM_REC_SIZE_DEFAULT       24
#define EIP160_XFORM_REC_WORD_COUNT         32
#define EIP160_REG_OFFS                     4
#define EIP160_REG_XFORM_REC_OFFSET         (EIP160_CONF_BASE + 0x0000)
#define EIP160_REG_XFORM_REC(n)             (EIP160_REG_XFORM_REC_OFFSET + \
                                             (0x10000 * (n >> 6)) + \
                                             (EIP160_XFORM_REC_WORD_COUNT * \
                                             EIP160_REG_OFFS * (n & 0x3f)))

/* Receiver Secure Channel (RxSC) lookup table (CAM) (ingress only) */
#define EIP160_REG_RXSC_CAM_OFFS            16
#define EIP160_REG_RXSC_CAM_SCI_LO(n)       (EIP160_CONF_BASE + 0x3400 + \
                                             (0x10000 * (n >> 6)) + \
                                             (EIP160_REG_RXSC_CAM_OFFS * \
                                              (n & 0x3f)))

#define EIP160_REG_RXSC_CAM_SCI_HI(n)       (EIP160_CONF_BASE + 0x3404 + \
                                             (0x10000 * (n >> 6)) + \
                                             (EIP160_REG_RXSC_CAM_OFFS * \
                                              (n & 0x3f)))

#define EIP160_REG_RXSC_CAM_CTRL(n)         (EIP160_CONF_BASE + 0x3408 + \
                                             (0x10000 * (n >> 6)) + \
                                             (EIP160_REG_RXSC_CAM_OFFS * \
                                              (n & 0x3f)))

#define  EIP160_REG_RXSC_ENTRY_ENABLE(n)    (EIP160_CONF_BASE + 0x3600 + \
                                             (EIP160_REG_OFFS * (n - 1)))

#define EIP160_REG_RXSC_ENTRY_ENABLE_CTRL   (EIP160_CONF_BASE + 0x3700)

/* Secure Channel (SC) to Security Association (SA) mapping, AN decoding */
#define EIP160_REG_SC_SA_MAP_OFFS           8
#define EIP160_REG_SC_SA_MAP1(n)            (EIP160_CONF_BASE + 0x3800 + \
                                             (0x10000 * (n >> 6)) + \
                                             (EIP160_REG_SC_SA_MAP_OFFS * \
                                              (n & 0x3f)))

#define EIP160_REG_SC_SA_MAP2(n)            (EIP160_CONF_BASE + 0x3804 + \
                                             (0x10000 * (n >> 6)) + \
                                             (EIP160_REG_SC_SA_MAP_OFFS * \
                                              (n & 0x3f)))

/* Input TCAM entries (n), */
/* each entry has (m) 32-bit words, for control word m=0 */
#define EIP160_REG_TCAM_ENTRY_OFFS          0x40 /* TCAM entry size in bytes */
#define EIP160_REG_TCAM_KEY_WORD_COUNT      8
#define EIP160_REG_TCAM_MASK_WORD_COUNT     EIP160_REG_TCAM_KEY_WORD_COUNT
#define EIP160_REG_TCAM_OFFSET              (EIP160_CONF_BASE + 0x2000)
#define EIP160_REG_TCAM_KEY(n,m)            (EIP160_REG_TCAM_OFFSET + \
                                             EIP160_REG_TCAM_ENTRY_OFFS * n + \
                                             EIP160_REG_OFFS * m)
#define EIP160_REG_TCAM_CTRL_KEY(n)         EIP160_REG_TCAM_KEY(n,0)
#define EIP160_REG_TCAM_MASK(n,m)           (EIP160_REG_TCAM_OFFSET + 0x20 + \
                                             EIP160_REG_TCAM_ENTRY_OFFS * n + \
                                             EIP160_REG_OFFS * m)
#define EIP160_REG_TCAM_CTRL_MASK(n)        EIP160_REG_TCAM_MASK(n,0)

/* TCAM policy */
#define EIP160_REG_TCAM_POLICY_OFFS         (EIP160_CONF_BASE + 0x3000)
#define EIP160_REG_TCAM_POLICY(n)           (EIP160_REG_TCAM_POLICY_OFFS + \
                                             (0x10000 * (n >> 6)) + \
                                             (EIP160_REG_OFFS * \
                                              (n & 0x3f)))

/* TCAM statistics counters: n - counter ID */
#define EIP160_REG_TCAM_STATISTICS_ID_OFFS          0x08
#define EIP160_REG_TCAM_STATISTICS_BANK_OFFS        0x10000 /* regs mirrored */
#define EIP160_REG_TCAM_STAT_COUNT_OFFS      (EIP160_CONF_BASE + 0x4000)
#define EIP160_REG_TCAM_STATISTICS(n)        (EIP160_CONF_BASE + \
                                    EIP160_REG_TCAM_STAT_COUNT_OFFS + \
                                    (EIP160_REG_TCAM_STATISTICS_BANK_OFFS * \
                                    (n >> 6)) + \
                                    (EIP160_REG_TCAM_STATISTICS_ID_OFFS * \
                                    (n & 0x3f)))

#define EIP160_TCAM_STAT_HIT                        0

/* RxCAM statistics counters: n - counter ID */
#define EIP160_REG_RXCAM_STATISTICS_ID_OFFS         0x08
#define EIP160_REG_RXCAM_STATISTICS_BANK_OFFS       0x10000 /* regs mirrored */
#define EIP160_REG_RXCAM_STATISTICS_OFFS     (EIP160_CONF_BASE + 0x5000)
#define EIP160_REG_RXCAM_STATISTICS(n)       (EIP160_CONF_BASE + \
                                    EIP160_REG_RXCAM_STATISTICS_OFFS + \
                                    (EIP160_REG_RXCAM_STATISTICS_BANK_OFFS * \
                                    (n >> 6)) + \
                                    (EIP160_REG_RXCAM_STATISTICS_ID_OFFS * \
                                    (n & 0x3f)))

#define EIP160_RXCAM_STAT_HIT                       0
                                    

#define EIP160_REG_SAM_ENTRY_SET(n)         (EIP160_CONF_BASE + 0x6080 + \
                                             EIP160_REG_OFFS * n)
#define EIP160_REG_SAM_ENTRY_CLEAR(n)       (EIP160_CONF_BASE + 0x60C0 + \
                                             EIP160_REG_OFFS * n)

#define EIP62_REG_CTX_CTRL                  (EIP62_CONF_BASE + 0x0008)
#define EIP62_REG_CTX_UPD_CTRL              (EIP62_CONF_BASE + 0x30)
#define EIP62_REG_TYPE                      (EIP62_CONF_BASE + 0x03F8)
#define EIP62_REG_VERSION                   (EIP62_CONF_BASE + 0x03FC)


#define EIP160_REG_SAF_CTRL_DEFAULT         0x00000000
#define EIP160_REG_RXSC_ENTRY_ENABLE_CTRL_DEFAULT   0x00000000
#define EIP160_REG_TCAM_CTRL_DEFAULT                0x00000000
#define EIP160_REG_TCAM_POLICY_DEFAULT              0x00000000
#define EIP160_REG_SC_SA_MAP1_DEFAULT               0x00000000
#define EIP160_REG_SC_SA_MAP2_DEFAULT               0x00000000
#define EIP160_REG_SAM_IN_FLIGHT_DEFAULT            0x00000000

#define EIP160_SECY_STAT_TRANSFORM_ERROR_PKTS       0
#define EIP160_SECY_STAT_E_OUT_PKTS_CTRL            1
#define EIP160_SECY_STAT_E_OUT_PKTS_UNTAGGED        2
#define EIP160_SECY_STAT_I_IN_PKTS_CTRL             1
#define EIP160_SECY_STAT_I_IN_PKTS_UNTAGGED         2
#define EIP160_SECY_STAT_I_IN_PKTS_NOTAG            3
#define EIP160_SECY_STAT_I_IN_PKTS_BADTAG           4
#define EIP160_SECY_STAT_I_IN_PKTS_NOSCI            5
#define EIP160_SECY_STAT_I_IN_PKTS_UNKNOWNSCI       6
#define EIP160_SECY_STAT_I_IN_PKTS_TAGGEDCTRL       7

#define EIP160_IFC_STAT_E_OUT_OCTETS_COMMON                     0
#define EIP160_IFC_STAT_E_OUT_OCTETS_UNCONTROLLED               1
#define EIP160_IFC_STAT_E_OUT_OCTETS_CONTROLLED                 2
#define EIP160_IFC_STAT_E_OUT_UCAST_PKTS_UNCONTROLLED           3
#define EIP160_IFC_STAT_E_OUT_MCAST_PKTS_UNCONTROLLED           4
#define EIP160_IFC_STAT_E_OUT_BCAST_PKTS_UNCONTROLLED           5
#define EIP160_IFC_STAT_E_OUT_UCAST_PKTS_CONTROLLED             6
#define EIP160_IFC_STAT_E_OUT_MCAST_PKTS_CONTROLLED             7
#define EIP160_IFC_STAT_E_OUT_BCAST_PKTS_CONTROLLED             8
#define EIP160_IFC_STAT_I_IN_OCTETS_UNCONTROLLED                0
#define EIP160_IFC_STAT_I_IN_OCTETS_CONTROLLED                  1
#define EIP160_IFC_STAT_I_IN_UCAST_PKTS_UNCONTROLLED            2
#define EIP160_IFC_STAT_I_IN_MCAST_PKTS_UNCONTROLLED            3
#define EIP160_IFC_STAT_I_IN_BCAST_PKTS_UNCONTROLLED            4
#define EIP160_IFC_STAT_I_IN_UCAST_PKTS_CONTROLLED              5
#define EIP160_IFC_STAT_I_IN_MCAST_PKTS_CONTROLLED              6
#define EIP160_IFC_STAT_I_IN_BCAST_PKTS_CONTROLLED              7

#define EIP160_SA_STAT_E_OUT_OCTETS_ENC_PROT        0
#define EIP160_SA_STAT_E_OUT_PKTS_ENC_PROT          1
#define EIP160_SA_STAT_E_OUT_PKTS_TOO_LONG          2
#define EIP160_SA_STAT_E_OUT_PKTS_SA_NOT_IN_USE     3
#define EIP160_SA_STAT_I_IN_OCTETS_DEC              0
#define EIP160_SA_STAT_I_IN_OCTETS_VALIDATED        1
#define EIP160_SA_STAT_I_IN_PKTS_UNCHECKED          2
#define EIP160_SA_STAT_I_IN_PKTS_DELAYED            3
#define EIP160_SA_STAT_I_IN_PKTS_LATE               4
#define EIP160_SA_STAT_I_IN_PKTS_OK                 5
#define EIP160_SA_STAT_I_IN_PKTS_INVALID            6
#define EIP160_SA_STAT_I_IN_PKTS_NOT_VALID          7
#define EIP160_SA_STAT_I_IN_PKTS_NOT_USING_SA       8
#define EIP160_SA_STAT_I_IN_PKTS_UNUSED_SA          9

#define EIP217_REG_COUNTER_LO_DEFAULT       0x00000000
#define EIP217_REG_COUNTER_HI_DEFAULT       0x00000000


/* Clock bitmask specification for clock control */
/* Packet transform engine */
#define EIP160_SECY_DEVICE_PE_CLOCK         BIT_0
/* Input Classification engine */
#define EIP160_SECY_DEVICE_ICE_CLOCK        BIT_1
/* TCAM */
#define EIP160_SECY_DEVICE_TCAM_CLOCK       BIT_3
/* RX CAM (Ingress-only, reserved for egress-only) */
#define EIP160_SECY_DEVICE_RXCAM_CLOCK      BIT_4
/* Flow registers */
#define EIP160_SECY_DEVICE_FLOW_CLOCK       BIT_5
/* All clocks bitmask */
#define EIP160_SECY_ALL_CLOCKS              EIP160_SECY_DEVICE_PE_CLOCK  | \
                                            EIP160_SECY_DEVICE_ICE_CLOCK | \
                                            EIP160_SECY_DEVICE_TCAM_CLOCK | \
                                            EIP160_SECY_DEVICE_RXCAM_CLOCK | \
                                            EIP160_SECY_DEVICE_FLOW_CLOCK

#define EIP160_RULE_NON_CTRL_WORD_COUNT 5
#define ADAPTER_EIP160_MAX_NOF_SYNC_RETRY_COUNT 100
#define SAB_MACSEC_SA_WORD_COUNT 24

/* Context control word values for MACsec */
#define SAB_CW0_MACSEC_EG32 0x9241e066
#define SAB_CW0_MACSEC_IG32 0xd241e06f
#define SAB_CW0_MACSEC_EG64 0xa241e066
#define SAB_CW0_MACSEC_IG64 0xe241a0ef

/* Context control word values for basic encryption/authentication */
#define SAB_CW0_ENCAUTH_AES_GCM  0x82018006
#define SAB_CW0_AUTHDEC_AES_GCM  0x8201800f
#define SAB_CW0_ENC_AES_CTR      0x80010004

/* Context control word values for various AES key lengths. */
#define SAB_CW0_AES128 0x000a0000
#define SAB_CW0_AES192 0x000c0000
#define SAB_CW0_AES256 0x000e0000

/* Various options for MACsec SAs */
#define SAB_CW0_ROLLOVER    0x00000400
#define SAB_CW0_KEEP_SECTAG 0x00000200
#define SAB_CW0_KEEP_ICV    0x00000100



/* XLMAC */
                                            /*
 *  64-bit value manipulation
 */
#ifndef _BYTE_ORDER_LITTLE_ENDIAN_ /* Big Endian */
  #define XLMAC_DATA_64_TO_32_HI(dst, src)  ((dst) = (uint32_t) (src))
  #define XLMAC_DATA_64_TO_32_LO(dst, src)  ((dst) = (uint32_t) ((src) >> 32))
  #define XLMAC_DATA_64_LO(src)             ((uint32_t) ((src) >> 32))
  #define XLMAC_DATA_64_HI(src)             ((uint32_t) (src))
  #define XLMAC_DATA_64_SET(dst, src_hi, src_lo)   \
          ((dst) = (((uint64_t) ((uint32_t)(src_lo))) << 32) | ((uint64_t) ((uint32_t)(src_hi))))
#else   /* Little Endian */
  #define XLMAC_DATA_64_TO_32_LO(dst, src)  ((dst) = (uint32_t) (src))
  #define XLMAC_DATA_64_TO_32_HI(dst, src)  ((dst) = (uint32_t) ((src) >> 32))
  #define XLMAC_DATA_64_HI(src)             ((uint32_t) ((src) >> 32))
  #define XLMAC_DATA_64_LO(src)             ((uint32_t) (src))
  #define XLMAC_DATA_64_SET(dst, src_hi, src_lo)   \
          ((dst) = (((uint64_t) ((uint32_t)(src_hi))) << 32) | ((uint64_t) ((uint32_t)(src_lo))))
#endif  /* Endian */

#define BIT_31_16_MASK             0xFFFF0000
#define BIT_15_00_MASK             0xFFFF
#define BIT_06_00_MASK             0x007F

#define XLMAC_REG_BASE                  0xF8000000
#define XLMAC_REG_ANONYM_BASE           0xF8FF0000
#define XLMAC_REG_XLPORT_LO_BASE        XLMAC_REG_ANONYM_BASE
#define XLMAC_REG_XLPORT_HI_BASE        XLMAC_REG_ANONYM_BASE

#define PHYMOD_REG_ACC_XLMAC_64_BIT     0
#define PHYMOD_REG_ACC_XLMAC            0

#define MACSEC_CONF_CTL_COMMITf      (1U << 10)  /* XLMAC config control : commit            */
#define MACSEC_CONF_CTL_WRITEf       (1U <<  9)  /* XLMAC config control : write operation   */
#define MACSEC_CONF_CTL_ADDRf        BIT_06_00_MASK  /* XLMAC config control : XLMAC reg. address */
#define MACSEC_CONF_CTL_PORTIDs      7           /* XLMAC config control : line or switch side    */
#define MACSEC_CONF_CTL_SW_SIDE      (0x2 <<  MACSEC_CONF_CTL_PORTIDs)  /* XLMAC_conf_ctl[8:7]=10b: SW side */
#define MACSEC_CONF_CTL_LN_SIDE      (0x0 <<  MACSEC_CONF_CTL_PORTIDs)  /* XLMAC_conf_ctl[8:7]=00b: line side */
#define MACSEC_CONF_DATA_HIr         0x1000  /* XLMAC data high register         */
#define MACSEC_CONF_DATA_LOr         0x1004  /* XLMAC data low  register         */
#define MACSEC_CONF_CTLr             0x1008  /* XLMAC config control register    */



#define XLMAC_CTRLr ((0x00000000 + XLMAC_REG_BASE) | PHYMOD_REG_ACC_XLMAC_64_BIT)
#define XLMAC_CTRLr_SIZE 8

typedef union 
{
  uint32_t v[2];
  uint32_t xlmac_ctrl[2];
  uint64_t _xlmac_ctrl;
} XLMAC_CTRLr_t;

#define XLMAC_CTRLr_CLR(r) memset(&(r), 0, sizeof(XLMAC_CTRLr_t))
#define XLMAC_CTRLr_SET(r,i,d) (r).xlmac_ctrl[i] = d
#define XLMAC_CTRLr_GET(r,i) (r).xlmac_ctrl[i]


#define XLMAC_MODEr ((0x00000001 + XLMAC_REG_BASE) | PHYMOD_REG_ACC_XLMAC_64_BIT)
#define XLMAC_MODEr_SIZE 8

typedef union 
{
  uint32_t v[2];
  uint32_t xlmac_mode[2];
  uint64_t _xlmac_mode;
} XLMAC_MODEr_t;

#define XLMAC_MODEr_CLR(r) memset(&(r), 0, sizeof(XLMAC_MODEr_t))
#define XLMAC_MODEr_SET(r,i,d) (r).xlmac_mode[i] = d
#define XLMAC_MODEr_GET(r,i) (r).xlmac_mode[i]


#define XLMAC_TX_CTRLr ((0x00000004 + XLMAC_REG_BASE) | PHYMOD_REG_ACC_XLMAC_64_BIT)
#define XLMAC_TX_CTRLr_SIZE 8

typedef union 
{
  uint32_t v[2];
  uint32_t xlmac_tx_ctrl[2];
  uint64_t _xlmac_tx_ctrl;
} XLMAC_TX_CTRLr_t;

#define XLMAC_TX_CTRLr_CLR(r) memset(&(r), 0, sizeof(XLMAC_TX_CTRLr_t))
#define XLMAC_TX_CTRLr_SET(r,i,d) (r).xlmac_tx_ctrl[i] = d
#define XLMAC_TX_CTRLr_GET(r,i) (r).xlmac_tx_ctrl[i]


#define XLMAC_RX_CTRLr ((0x00000006 + XLMAC_REG_BASE) | PHYMOD_REG_ACC_XLMAC_64_BIT)
#define XLMAC_RX_CTRLr_SIZE 8

typedef union 
{
  uint32_t v[2];
  uint32_t xlmac_rx_ctrl[2];
  uint64_t _xlmac_rx_ctrl;
} XLMAC_RX_CTRLr_t;

#define XLMAC_RX_CTRLr_CLR(r) memset(&(r), 0, sizeof(XLMAC_RX_CTRLr_t))
#define XLMAC_RX_CTRLr_SET(r,i,d) (r).xlmac_rx_ctrl[i] = d
#define XLMAC_RX_CTRLr_GET(r,i) (r).xlmac_rx_ctrl[i]


#define XLMAC_CTRLr_RX_ENf_SET(r,f) (r).xlmac_ctrl[0]=(((r).xlmac_ctrl[0] & ~((uint32_t)0x1 << 1)) | ((((uint32_t)f) & 0x1) << 1)) 
#define XLMAC_CTRLr_TX_ENf_SET(r,f) (r).xlmac_ctrl[0]=(((r).xlmac_ctrl[0] & ~((uint32_t)0x1)) | (((uint32_t)f) & 0x1)) 
#define XLMAC_MODEr_SPEED_MODEf_SET(r,f) (r).xlmac_mode[0]=(((r).xlmac_mode[0] & ~((uint32_t)0x7 << 4)) | ((((uint32_t)f) & 0x7) << 4)) | (7 << (16 + 4))
#define XLMAC_TX_CTRLr_TX_THRESHOLDf_SET(r,f) (r).xlmac_tx_ctrl[1]=(((r).xlmac_tx_ctrl[1] & ~((uint32_t)0xf << 6)) | ((((uint32_t)f) & 0xf) << 6)) 
#define XLMAC_TX_CTRLr_TX_PREAMBLE_LENGTHf_SET(r,f) (r).xlmac_tx_ctrl[1]=(((r).xlmac_tx_ctrl[1] & ~((uint32_t)0xf << 1)) | ((((uint32_t)f) & 0xf) << 1)) 
#define XLMAC_TX_CTRLr_AVERAGE_IPGf_SET(r,f) (r).xlmac_tx_ctrl[0]=(((r).xlmac_tx_ctrl[0] & ~((uint32_t)0x7f << 12)) | ((((uint32_t)f) & 0x7f) << 12))
#define XLMAC_TX_CTRLr_PAD_THRESHOLDf_SET(r,f) (r).xlmac_tx_ctrl[0]=(((r).xlmac_tx_ctrl[0] & ~((uint32_t)0x7f << 5)) | ((((uint32_t)f) & 0x7f) << 5))
#define XLMAC_TX_CTRLr_PAD_ENf_SET(r,f) (r).xlmac_tx_ctrl[0]=(((r).xlmac_tx_ctrl[0] & ~((uint32_t)0x1 << 4)) | ((((uint32_t)f) & 0x1) << 4))
#define XLMAC_TX_CTRLr_CRC_MODEf_SET(r,f) (r).xlmac_tx_ctrl[0]=(((r).xlmac_tx_ctrl[0] & ~((uint32_t)0x3)) | (((uint32_t)f) & 0x3))
#define XLMAC_RX_CTRLr_RX_PASS_PFCf_SET(r,f) (r).xlmac_rx_ctrl[0]=(((r).xlmac_rx_ctrl[0] & ~((uint32_t)0x1 << 15)) | ((((uint32_t)f) & 0x1) << 15)) | (1 << (16 + 15))
#define XLMAC_RX_CTRLr_RX_PASS_PAUSEf_SET(r,f) (r).xlmac_rx_ctrl[0]=(((r).xlmac_rx_ctrl[0] & ~((uint32_t)0x1 << 14)) | ((((uint32_t)f) & 0x1) << 14)) | (1 << (16 + 14))
#define XLMAC_RX_CTRLr_RX_PASS_CTRLf_SET(r,f) (r).xlmac_rx_ctrl[0]=(((r).xlmac_rx_ctrl[0] & ~((uint32_t)0x1 << 13)) | ((((uint32_t)f) & 0x1) << 13)) | (1 << (16 + 13))
#define XLMAC_RX_CTRLr_RUNT_THRESHOLDf_SET(r,f) (r).xlmac_rx_ctrl[0]=(((r).xlmac_rx_ctrl[0] & ~((uint32_t)0x7f << 4)) | ((((uint32_t)f) & 0x7f) << 4)) | (127 << (16 + 4))
#define XLMAC_RX_CTRLr_STRICT_PREAMBLEf_SET(r,f) (r).xlmac_rx_ctrl[0]=(((r).xlmac_rx_ctrl[0] & ~((uint32_t)0x1 << 3)) | ((((uint32_t)f) & 0x1) << 3)) | (1 << (16 + 3))


#define XLMAC_PFC_CTRLr ((0x0000000e + XLMAC_REG_BASE) | PHYMOD_REG_ACC_XLMAC_64_BIT)
#define XLMAC_PFC_CTRLr_SIZE 8

typedef union 
{
  uint32_t v[2];
  uint32_t xlmac_pfc_ctrl[2];
  uint64_t _xlmac_pfc_ctrl;
} XLMAC_PFC_CTRLr_t;

#define XLMAC_PFC_CTRLr_CLR(r) memset(&(r), 0, sizeof(XLMAC_PFC_CTRLr_t))
#define XLMAC_PFC_CTRLr_SET(r,i,d) (r).xlmac_pfc_ctrl[i] = d
#define XLMAC_PFC_CTRLr_GET(r,i) (r).xlmac_pfc_ctrl[i]

#define XLMAC_PFC_CTRLr_PFC_REFRESH_ENf_SET(r,f) (r).xlmac_pfc_ctrl[1]=(((r).xlmac_pfc_ctrl[1] & ~((uint32_t)0x1)) | (((uint32_t)f) & 0x1))


#define XLMAC_RX_MAX_SIZEr ((0x00000008 + XLMAC_REG_BASE) | PHYMOD_REG_ACC_XLMAC_64_BIT)
#define XLMAC_RX_MAX_SIZEr_SIZE 8

typedef union 
{
  uint32_t v[2];
  uint32_t xlmac_rx_max_size[2];
  uint64_t _xlmac_rx_max_size;
} XLMAC_RX_MAX_SIZEr_t;

#define XLMAC_RX_MAX_SIZEr_CLR(r) memset(&(r), 0, sizeof(XLMAC_RX_MAX_SIZEr_t))
#define XLMAC_RX_MAX_SIZEr_SET(r,i,d) (r).xlmac_rx_max_size[i] = d
#define XLMAC_RX_MAX_SIZEr_GET(r,i) (r).xlmac_rx_max_size[i]

#define XLMAC_RX_MAX_SIZEr_RX_MAX_SIZEf_SET(r,f) (r).xlmac_rx_max_size[0]=(((r).xlmac_rx_max_size[0] & ~((uint32_t)0x3fff)) | (((uint32_t)f) & 0x3fff)) | (0x3fff << 16)

#endif /* PHY_MAC_SEC_DEFS_H_ */
