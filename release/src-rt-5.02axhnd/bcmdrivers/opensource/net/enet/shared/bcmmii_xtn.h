
/*
 Copyright 2004-2010 Broadcom Corp. All Rights Reserved.

 <:label-BRCM:2011:DUAL/GPL:standard

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
#ifndef _BCMMII_XTN_H_
#define _BCMMII_XTN_H_

#if defined(CONFIG_BCM963158)
    #define PBMAP_P7_IMP                        (1<<P7_PORT_ID)
    #define PBMAP_P5_IMP                        (1<<P5_PORT_ID)

    /* SWITCH_REG_SWITCH_CNTRL register definitions */
    #define SF2_P8_GMII_MASK                    (3UL<<3)
    #define SF2_P7_GMII_MASK                    (3UL<<5)
    #define SF2_P6_GMII_MASK                    (3UL<<7)
    #define SF2_P5_GMII_MASK                    (3UL<<9)

    #define SF2_P8_GMII_3G                      (3<<3)
    #define SF2_P8_GMII_2_5G                    (2<<3)
    #define SF2_P8_GMII_2G                      (1<<3)
    #define SF2_P8_GMII_1G                      (0<<3)
    #define SF2_P7_GMII_3G                      (3<<5)
    #define SF2_P7_GMII_2_5G                    (2<<5)
    #define SF2_P7_GMII_2G                      (1<<5)
    #define SF2_P7_GMII_1G                      (0<<5)
    #define SF2_P6_GMII_3G                      (3<<7)
    #define SF2_P6_GMII_2_5G                    (2<<7)
    #define SF2_P6_GMII_2G                      (1<<7)
    #define SF2_P6_GMII_1G                      (0<<7)
    #define SF2_P5_GMII_3G                      (3<<9)
    #define SF2_P5_GMII_2_5G                    (2<<9)
    #define SF2_P5_GMII_2G                      (1<<9)
    #define SF2_P5_GMII_1G                      (0<<9)

    #define DEFAULT_IMP_SPEEDS                  (SF2_P8_GMII_2_5G | SF2_P7_GMII_2G | SF2_P5_GMII_3G)
    #define IMP_SPEED_MASK                      (SF2_P8_GMII_MASK | SF2_P7_GMII_MASK | SF2_P5_GMII_MASK)
    
#else   // !BCM963158
#define SF2_MDIO_MASTER                     0x01

#define SF2_P8_2_5G_EN                      (1<<5)

    #if defined(CONFIG_BCM_ENET_MULTI_IMP_SUPPORT)
    /* Note : PBMAP_P4/P5_IMP is only defined when multiple IMP ports are in use
     * Should not be used as a replacement for PBMAP_P4/P5 */
    #define PBMAP_P5_IMP                        (1<<P5_PORT_ID)
    #define PBMAP_P4_IMP                        (1<<P4_PORT_ID)
    #define SF2_P5_2_5G_EN                      (1<<7)
    #define SF2_P4_HS_SEL                       (1<<14)
    #else
    #define PBMAP_P5_IMP                        (0)
    #define PBMAP_P4_IMP                        (0)
    #define SF2_P5_2_5G_EN                      (0) /* Only supported in 4908 */
    #define SF2_P4_HS_SEL                       (0) /* Only supported in 4908 */
    #endif

    #define SF2_IMP_2_5G_EN                     ( SF2_P8_2_5G_EN | SF2_P5_2_5G_EN | SF2_P4_HS_SEL ) 
#endif // !BCM963158


/* These are needed for register accesses of length >4 bytes */

enum {
    SF2_P5  = 5,
    SF2_P7  = 7,
    SF2_P11 = 11,
    SF2_P12 = 12,
    SF2_INEXISTANT_PORT = 6,
};

#define PAGE_SWITCH_EXTEND_REG     0x100

#if defined(CONFIG_BCM94908)
#define SF2_P11_RGMII_CTRL_REGS             0x53UL
#define SF2_P11_RGMII_RX_CLK_DELAY_CTRL     0x55UL
#else
//  SF2 P5 RGMII Control Register
#define SF2_P5_RGMII_CTRL_REGS              0x1cUL
#define SF2_P5_RGMII_RX_CLK_DELAY_CTRL      0x1eUL
//  SF2 P7 RGMII Control Register
#define SF2_P7_RGMII_CTRL_REGS              0x1fUL
#define SF2_P7_RGMII_RX_CLK_DELAY_CTRL      0x21UL
//  SF2 P11 RGMII Control Register
#define SF2_P11_RGMII_CTRL_REGS             0x32UL
#define SF2_P11_RGMII_RX_CLK_DELAY_CTRL     0x34UL
//  SF2 P12 RGMII Control Register
#define SF2_P12_RGMII_CTRL_REGS             0x35UL
#define SF2_P12_RGMII_RX_CLK_DELAY_CTRL     0x37UL
#endif /* !4908 */

#define SF2_QUAD_PHY_BASE_REG               0x40024UL  // default quad phy adr base = 1
#define SF2_QUAD_PHY_SYSTEM_RESET           0x100
#define SF2_QUAD_PHY_PHYAD_SHIFT            12

#define SF2_ENABLE_PORT_RGMII_INTF          0x01
#define SF2_TX_ID_DIS                       0x02
#define SF2_RGMII_PORT_MODE_M               0x1C
#define SF2_RGMII_PORT_MODE_S               0x2
  #define SF2_RGMII_PORT_MODE_INT_EPHY_MII      0x0 /* Internal EPHY (MII) */
  #define SF2_RGMII_PORT_MODE_INT_GPHY_GMII     0x1 /* Internal GPHY (GMII/MII) */
  #define SF2_RGMII_PORT_MODE_EXT_EPHY_MII      0x2 /* External EPHY (MII) */
  #define SF2_RGMII_PORT_MODE_EXT_GPHY_RGMII    0x3 /* External GPHY (RGMII) */
  #define SF2_RGMII_PORT_MODE_EXT_RvMII         0x4 /* External RvMII */
#define SF2_RGMII_RX_CLK_IDDQ               0x10
#define SF2_RX_ID_BYPASS                    0x20
#define SF2_MISC_MII_PAD_CTL                (MISC_BASE + 0x28)

#if defined(CONFIG_5x3_CROSSBAR_SUPPORT) /* 63138B0 onwards 5x3 crossbar */
#define CB_PHY_PORT_MASK                    0x7
#define CB_PHY_PORT_SHIFT                   0x3
#define CB_WAN_LNK_STATUS_SHIFT             9
#define CB_WAN_LNK_STATUS_MASK              (1<<CB_WAN_LNK_STATUS_SHIFT)
#define CB_WAN_LNK_STATUS_SRC_SHIFT         10
#define CB_WAN_LNK_STATUS_SRC_MASK          (1<<CB_WAN_LNK_STATUS_SRC_SHIFT)
#elif defined CONFIG_3x2_CROSSBAR_SUPPORT /* 4908 3x2 crossbar */
#define CB_PHY_PORT_MASK                    0x3
#define CB_PHY_PORT_SHIFT                   0x2
#define CB_WAN_LNK_STATUS_SHIFT             4
#define CB_WAN_LNK_STATUS_MASK              (1<<CB_WAN_LNK_STATUS_SHIFT)
#define CB_WAN_LNK_STATUS_SRC_SHIFT         5
#define CB_WAN_LNK_STATUS_SRC_MASK          (1<<CB_WAN_LNK_STATUS_SRC_SHIFT)
#else /* 4x2_CROSSBAR_SUPPORT */
#define CB_PHY_PORT_MASK                    0x3
#define CB_PHY_PORT_SHIFT                   0x2
#if defined(CONFIG_BCM963148)
#define CB_WAN_LNK_STATUS_SHIFT             4
#define CB_WAN_LNK_STATUS_MASK              (1<<CB_WAN_LNK_STATUS_SHIFT)
#define CB_WAN_LNK_STATUS_SRC_SHIFT         5   /* Dummy Constant */
#define CB_WAN_LNK_STATUS_SRC_MASK          (0<<CB_WAN_LNK_STATUS_SRC_SHIFT)
#endif
#endif
#define SF2_MDIO_COMMAND_REG                (SWITCH_MDIO_BASE)
    #define SF2_MDIO_BUSY                       (1 << 29)
    #define SF2_MDIO_FAIL                       (1 << 28)
    #define SF2_MDIO_CMD_S                      26
    #define SF2_MDIO_CMD_M                      (3<<SF2_MDIO_CMD_S)
    #define SF2_MDIO_CMD_ADDR_C45               (0<<SF2_MDIO_CMD_S)
    #define SF2_MDIO_CMD_WRITE                  (1<<SF2_MDIO_CMD_S)
    #define SF2_MDIO_CMD_C22_READ               (2<<SF2_MDIO_CMD_S)
    #define SF2_MDIO_CMD_C45_READ_INC           (2<<SF2_MDIO_CMD_S)
    #define SF2_MDIO_CMD_C45_READ               (3<<SF2_MDIO_CMD_S)
    #define SF2_MDIO_PHY_PORT_ADDR_S             21
    #define SF2_MDIO_PHY_PORT_ADDR_M            (0x1f<<SF2_MDIO_PHY_PORT_ADDR_S)
    #define SF2_MDIO_PHY_REG_DEV_S              16
    #define SF2_MDIO_PHY_REG_DEV_M              (0x1f<<SF2_MDIO_PHY_REG_DEV_S)
    #define SF2_MDIO_PHY_ADDR_DATA_M            0xffff

#define SF2_MDIO_CONFIG_REG                 (SWITCH_MDIO_BASE + 0x00004UL)
    #define SF2_MDIO_CONFIG_SUPRESS_PREAMBLE    (1<<12)
    #define SF2_MDIO_CONFIG_CLK_DIVIDER_SHFT    4
    #define SF2_MDIO_CONFIG_CLK_DIVIDER_MASK    (0x3f << SF2_MDIO_CONFIG_CLK_DIVIDER_SHFT)
    #define SF2_MDIO_CONFIG_CLAUSE22            1

#define SF2_IMP0_PORT                       8
#define SF2_WAN_IMP1_PORT                   5

#define SF2_ACB_CONTROL_REG                 SWITCH_ACB_BASE
    #define SF2_ACB_EN                      1
#if defined(ACB_ALGORITHM2)
    #define SF2_ACB_ALGORITHM_S             1
    #define SF2_ACB_ALGORITHM_M             0x1
    #define SF2_ACB_FLUSH_S                 2
    #define SF2_ACB_FLUSH_M                 0x7
    #define SF2_ACB_EOP_DELAY_S             5
    #define SF2_ACB_EOP_DELAY_M             0xff
#endif
#define SF2_ACB_XON_THRESH_REG              (SWITCH_ACB_BASE + 0x00004UL)
    #define SF2_ACB_BUFS_THRESH_M           0x7FF
    #define SF2_ACB_TOTAL_XON_BUFS_S        11
    #define SF2_ACB_XON_BUFS_S          0
#define SF2_ACB_QUE0_CONF_REG               (SWITCH_ACB_BASE + 0x00008UL)
    #define SF2_ACB_QUE_PESSIMISTIC_M       1
    #define SF2_ACB_QUE_PESSIMISTIC_S       31
    #define SF2_ACB_QUE_PKT_LEN_M           0x3F
    #define SF2_ACB_QUE_PKT_LEN_S           25
    #define SF2_ACB_QUE_TOTAL_XON_M         1
    #define SF2_ACB_QUE_TOTAL_XON_S         24
    #define SF2_ACB_QUE_TOTAL_XOFF_M        1
    #define SF2_ACB_QUE_TOTAL_XOFF_S        23
    #define SF2_ACB_QUE_XON_M               1
    #define SF2_ACB_QUE_XON_S               11
    #define SF2_ACB_QUE_TOTOAL_XOFF_BUFS_S   12
    #define SF2_ACB_QUE_XOFF_BUFS_S         0
    #define SF2_ACB_QUE_MAX                 63
#if defined(ACB_ALGORITHM2)
#define SF2_ACB_QUE0_PKTS_IN_FLIGHT         (SWITCH_ACB_BASE + 0x00108UL)
    #define SF2_ACB_QUE_PKTS_IN_FLIGHT_M    0x7ff
#endif

/****************************************************************************
   Control_Page : Page (0x0)
****************************************************************************/
//#define REG_FAST_AGING_CTRL                           0x88
    #define FAST_AGE_MCAST                              0x20

//#define REG_CONTROL_MII1_PORT_STATE_OVERRIDE          0x0e
#define  IMP_PORT_SPEED_UP_2G                           0xc0

/****************************************************************************
   Control_Page : Page (0x2)
****************************************************************************/
    #define REG_BRCM_HDR_CTRL2                            0x0a

        #define BRCM_HDR_EN_P0                            (1<<0)
        #define BRCM_HDR_EN_P1                            (1<<1)
        #define BRCM_HDR_EN_P2                            (1<<2)
        #define BRCM_HDR_EN_P3                            (1<<3)
        #define BRCM_HDR_EN_P4                            (1<<4)

/****************************************************************************
   Flow Control: Page (0x0A)
****************************************************************************/
#define PAGE_FLOW_CTRL_XTN                                    0x0A

#define REG_FC_DIAG_CTRL                                      0x0
    #define FC_DIAG_PORT_MASK                                 0xf
    #define FC_DIAG_PORT_SHIFT                                0x0
#define REG_FC_CTRL_MODE                                      0x2
    #define FC_CTRL_MODE_PORT                                 0x1
#define REG_FC_CTRL_PORT_SEL                                  0x3
    #define REG_FC_CTRL_PORT_P0                               0x0
    #define REG_FC_CTRL_PORT_P1                               0x1
    #define REG_FC_CTRL_PORT_P2                               0x2
    #define REG_FC_CTRL_PORT_P3                               0x3
    #define REG_FC_CTRL_PORT_P4                               0x4
    #define REG_FC_CTRL_PORT_P8                               0x8
    #define REG_FC_CTRL_PORT_P7                               0x7
    #define REG_FC_CTRL_PORT_P5                               0x5
#define REG_FC_OOB_EN                                         0x4  // 16 bit
    #define FC_CTRL_OOB_EN_PORT_P5                            0x10
    #define FC_CTRL_OOB_EN_PORT_P7                            0x80
    #define FC_CTRL_OOB_EN_PORT_P8                            0x100
#define REG_FC_OOB_EN                                         0x4  // 16 bit
#define REG_FC_PAUSE_TIME_MAX                                 0x10  // 16 bit
#define REG_FC_PAUSE_TIME_MIN                                 0x12  // 16 bit
#define REG_FC_PAUSE_TIME_RESET_THD                           0x14  // 16 bit
#define REG_FC_PAUSE_TIME_DEFAULT                             0x18  // 16 bit
#define REG_FC_PAUSE_DROP_CTRL                                0x1c  // 16 bit
    #define FC_QUEUE_BASED_PAUSE_EN                           0x1000
    #define FC_TX_IMP0_TOTAL_PAUSE_EN                         0x800
    #define FC_TX_IMP0_TXQ_PAUSE_EN                           0x400
    #define FC_TX_IMP1_TOTAL_PAUSE_EN                         0x200
    #define FC_TX_IMP1_TXQ_PAUSE_EN                           0x100
    #define FC_TX_TOTAL_PAUSE_EN                              0x80
    #define FC_TX_TXQ_PAUSE_EN                                0x40
    #define FC_RX_DROP_EN                                     0x20
    #define FC_TX_TOTAL_DROP_EN                               0x10
    #define FC_TX_TXQ_DROP_EN                                 0x8
    #define FC_RX_BASED_CTRL_EN                               0x4
    #define FC_TX_QUANTUM_CTRL_EN                             0x2
    #define FC_TX_BASED_CTRL_EN                               0x1

    #define FC_LAN_TXQ_QUEUES                                 8

#define REG_FC_QUE_CUR_COUNT                0x30
#define REG_FC_QUE_PEAK_COUNT               0x40
#define REG_FC_SYS_TOTAL_PEAK_COUNT         0x50
#define REG_FC_SYS_TOTAL_USED_COUNT         0x52

#define REG_FC_PORT_PEAK_RX_BUFFER          0x54
#define REG_FC_QUE_FINAL_CONG_STAT          0x60
    #define FC_QUE_CONG_STAT_MASK           0x03
#define REG_FC_PORT_PAUSE_HISTORY           0x78
#define REG_FC_PORT_PAUSE_QUAN_HISTORY      0x7a

#define REG_FC_PORT_RXBASE_PAUSE_HISTORY    0x7c
#define REG_FC_PORT_RX_BUFFER_ERR_HISTORY   0x7e
#define REG_FC_QUE_CONG_STATUS              0x80
#define REG_FC_QUE_TOTAL_CONG_STATUS        0x9a

/************************************************************
 * SF2 Queue Hardware Constances                            *
 ************************************************************/
    #define SF2_MAX_BUFFER_IN_PAGE          0x5ff
    #define SF2_BYTES_PER_PAGE              0x100

/************************************************************
 * SF2 Flow Control Hardware Reset Default Threshold Values *
 * List the values as software configuration reference      *
 ************************************************************/
    /* LAN Ports Hardware Default Value */
    #define SF2_FC_PORT_RSV_THRE_HARD_DEF           0x18

    #define SF2_FC_PORT_HYST_THRE_HARD_DEF_Q0       0x4b
    #define SF2_FC_PORT_HYST_THRE_HARD_DEF_Q1       0x4f
    #define SF2_FC_PORT_HYST_THRE_HARD_DEF_Q2       0x53
    #define SF2_FC_PORT_HYST_THRE_HARD_DEF_Q3       0x57
    #define SF2_FC_PORT_HYST_THRE_HARD_DEF_Q4       0x5b
    #define SF2_FC_PORT_HYST_THRE_HARD_DEF_Q5       0x5f
    #define SF2_FC_PORT_HYST_THRE_HARD_DEF_Q6       0x63
    #define SF2_FC_PORT_HYST_THRE_HARD_DEF_Q7       0x67

    #define SF2_FC_PORT_PAUS_THRE_HARD_DEF_Q0       0x97
    #define SF2_FC_PORT_PAUS_THRE_HARD_DEF_Q1       0x9f
    #define SF2_FC_PORT_PAUS_THRE_HARD_DEF_Q2       0xa7
    #define SF2_FC_PORT_PAUS_THRE_HARD_DEF_Q3       0xaf
    #define SF2_FC_PORT_PAUS_THRE_HARD_DEF_Q4       0xb7
    #define SF2_FC_PORT_PAUS_THRE_HARD_DEF_Q5       0xbf
    #define SF2_FC_PORT_PAUS_THRE_HARD_DEF_Q6       0xc7
    #define SF2_FC_PORT_PAUS_THRE_HARD_DEF_Q7       0xcf

    #define SF2_FC_PORT_DROP_THRE_HARD_DEF_Q0_5     0x5cf
    #define SF2_FC_PORT_DROP_THRE_HARD_DEF_Q6_7     0x5ff

    #define SF2_FC_TOTL_HYST_THRE_HARD_DEF_Q0_7     0x38

    #define SF2_FC_TOTL_PAUS_THRE_HARD_DEF_Q0       0x40f
    #define SF2_FC_TOTL_PAUS_THRE_HARD_DEF_Q1       0x417
    #define SF2_FC_TOTL_PAUS_THRE_HARD_DEF_Q2       0x41f
    #define SF2_FC_TOTL_PAUS_THRE_HARD_DEF_Q3       0x427
    #define SF2_FC_TOTL_PAUS_THRE_HARD_DEF_Q4       0x42f
    #define SF2_FC_TOTL_PAUS_THRE_HARD_DEF_Q5       0x437
    #define SF2_FC_TOTL_PAUS_THRE_HARD_DEF_Q6       0x43f
    #define SF2_FC_TOTL_PAUS_THRE_HARD_DEF_Q7       0x447

    #define SF2_FC_TOTL_DROP_THRE_HARD_DEF_Q0       0x58f
    #define SF2_FC_TOTL_DROP_THRE_HARD_DEF_Q1       0x597
    #define SF2_FC_TOTL_DROP_THRE_HARD_DEF_Q2       0x59f
    #define SF2_FC_TOTL_DROP_THRE_HARD_DEF_Q3       0x5a7
    #define SF2_FC_TOTL_DROP_THRE_HARD_DEF_Q4       0x5af
    #define SF2_FC_TOTL_DROP_THRE_HARD_DEF_Q5       0x5b7
    #define SF2_FC_TOTL_DROP_THRE_HARD_DEF_Q6       0x5bf
    #define SF2_FC_TOTL_DROP_THRE_HARD_DEF_Q7       0x5c7

    /* IMP0 Hardware Default Value */
    #define SF2_FC_IMP0_RSVD_THRE_HARD_DEF_Q0_7     0x18

    #define SF2_FC_IMP0_HYST_THRE_HARD_DEF_Q0       0x63
    #define SF2_FC_IMP0_HYST_THRE_HARD_DEF_Q1       0x67
    #define SF2_FC_IMP0_HYST_THRE_HARD_DEF_Q2       0x6b
    #define SF2_FC_IMP0_HYST_THRE_HARD_DEF_Q3       0x6f
    #define SF2_FC_IMP0_HYST_THRE_HARD_DEF_Q4       0x73
    #define SF2_FC_IMP0_HYST_THRE_HARD_DEF_Q5       0x77
    #define SF2_FC_IMP0_HYST_THRE_HARD_DEF_Q6       0x7b
    #define SF2_FC_IMP0_HYST_THRE_HARD_DEF_Q7       0x7f

    #define SF2_FC_IMP0_PAUS_THRE_HARD_DEF_Q0       0xc7  
    #define SF2_FC_IMP0_PAUS_THRE_HARD_DEF_Q1       0xcf  
    #define SF2_FC_IMP0_PAUS_THRE_HARD_DEF_Q2       0xd7  
    #define SF2_FC_IMP0_PAUS_THRE_HARD_DEF_Q3       0xdf  
    #define SF2_FC_IMP0_PAUS_THRE_HARD_DEF_Q4       0xe7  
    #define SF2_FC_IMP0_PAUS_THRE_HARD_DEF_Q5       0xef  
    #define SF2_FC_IMP0_PAUS_THRE_HARD_DEF_Q6       0xf7  
    #define SF2_FC_IMP0_PAUS_THRE_HARD_DEF_Q7       0xff  

    #define SF2_FC_IMP0_DROP_THRE_HARD_DEF_Q0_7     0x5ff

    #define SF2_FC_IMP0_TTL_HYST_THRE_HARD_DEF_Q0_7     0x3bf

    #define SF2_FC_IMP0_TTL_PAUS_THRE_HARD_DEF_Q0       0x43f
    #define SF2_FC_IMP0_TTL_PAUS_THRE_HARD_DEF_Q1       0x447
    #define SF2_FC_IMP0_TTL_PAUS_THRE_HARD_DEF_Q2       0x44f
    #define SF2_FC_IMP0_TTL_PAUS_THRE_HARD_DEF_Q3       0x457
    #define SF2_FC_IMP0_TTL_PAUS_THRE_HARD_DEF_Q4       0x45f
    #define SF2_FC_IMP0_TTL_PAUS_THRE_HARD_DEF_Q5       0x467
    #define SF2_FC_IMP0_TTL_PAUS_THRE_HARD_DEF_Q6       0x46f
    #define SF2_FC_IMP0_TTL_PAUS_THRE_HARD_DEF_Q7       0x477

    #define SF2_FC_IMP0_TTL_DROP_THRE_HARD_DEF_Q0       0x5bf
    #define SF2_FC_IMP0_TTL_DROP_THRE_HARD_DEF_Q1       0x5c7
    #define SF2_FC_IMP0_TTL_DROP_THRE_HARD_DEF_Q2       0x5cf
    #define SF2_FC_IMP0_TTL_DROP_THRE_HARD_DEF_Q3       0x5d7
    #define SF2_FC_IMP0_TTL_DROP_THRE_HARD_DEF_Q4       0x5df
    #define SF2_FC_IMP0_TTL_DROP_THRE_HARD_DEF_Q5       0x5e7
    #define SF2_FC_IMP0_TTL_DROP_THRE_HARD_DEF_Q6       0x5ef
    #define SF2_FC_IMP0_TTL_DROP_THRE_HARD_DEF_Q7       0x5f7

/****************************************************************************
   Flow Control: Page (0x0B)
****************************************************************************/
#define FC_THRED_TOTAL_TYPES 7
#define FC_THRED_QUE_RSRVD_TYPE  0
#define FC_THRED_QUE_HYSTR_TYPE  1
#define FC_THRED_QUE_PAUSE_TYPE  2
#define FC_THRED_QUE_DROP_TYPE   3
#define FC_THRED_TTL_HYSTR_TYPE  4
#define FC_THRED_TTL_PAUSE_TYPE  5
#define FC_THRED_TTL_DROP_TYPE   6

#define PAGE_FC_LAN_TXQ                                       0x0B

    #define REG_FC_LAN_TXQ_THD_RSV_QN0                0x0   // 16 bits x 8 queues
    #define REG_FC_LAN_TXQ_THD_HYST_QN0               0x10  // 16 bits x 8 queues
    #define REG_FC_LAN_TXQ_THD_PAUSE_QN0              0x20  // 16 bits x 8 queues
    #define REG_FC_LAN_TXQ_THD_DROP_QN0               0x30  // 16 bits x 8 queues

    #define REG_FC_LAN_TOTAL_THD_HYST_QN0             0x40  // 16 bits x 8 queues
    #define REG_FC_LAN_TOTAL_THD_PAUSE_QN0            0x50  // 16 bits x 8 queues
    #define REG_FC_LAN_TOTAL_THD_DROP_QN0             0x60  // 16 bits x 8 queues
/****************************************************************************
   Flow Control IMP0: Page (0x0D)
****************************************************************************/
#define PAGE_FC_IMP0_TXQ                                       0x0D

    #define REG_FC_IMP0_TXQ_THD_RSV_QN0                0x0   // 16 bits x 8 queues
    #define REG_FC_IMP0_TXQ_THD_HYST_QN0               0x10  // 16 bits x 8 queues
    #define REG_FC_IMP0_TXQ_THD_PAUSE_QN0              0x20  // 16 bits x 8 queues
    #define REG_FC_IMP0_TXQ_THD_DROP_QN0               0x30  // 16 bits x 8 queues

    #define REG_FC_IMP0_TOTAL_THD_HYST_QN0             0x40  // 16 bits x 8 queues
    #define REG_FC_IMP0_TOTAL_THD_PAUSE_QN0            0x50  // 16 bits x 8 queues
    #define REG_FC_IMP0_TOTAL_THD_DROP_QN0             0x60  // 16 bits x 8 queues
/****************************************************************************
   Flow Control IMP1: Page (0x0E)
****************************************************************************/
#define PAGE_FC_IMP1_TXQ                                       0x0E

    #define REG_FC_IMP1_TXQ_THD_RSV_QN0                0x0   // 16 bits x 8 queues
    #define REG_FC_IMP1_TXQ_THD_HYST_QN0               0x10  // 16 bits x 8 queues
    #define REG_FC_IMP1_TXQ_THD_PAUSE_QN0              0x20  // 16 bits x 8 queues
    #define REG_FC_IMP1_TXQ_THD_DROP_QN0               0x30  // 16 bits x 8 queues

    #define REG_FC_IMP1_TOTAL_THD_HYST_QN0             0x40  // 16 bits x 8 queues
    #define REG_FC_IMP1_TOTAL_THD_PAUSE_QN0            0x50  // 16 bits x 8 queues
    #define REG_FC_IMP1_TOTAL_THD_DROP_QN0             0x60  // 16 bits x 8 queues
// use the following enums to retun GET result of sf2_pause_drop_ctrl()
enum {
    LAN,
    IMP0,
    IMP1,
};
/****************************************************************************
   MIB Counters: Page (0x20 to 0x28)
****************************************************************************/

//#define PAGE_MIB_P0                                       0x20

/* NOTE : TBD ; Almost all (except Q6/7 and TX size based octet count) of these stats
 * are applicable to other External switches as well; should remove duplicity  */

    #define SF2_REG_MIB_P0_TXOCTETS                           0x00
    #define SF2_REG_MIB_P0_TXDROPS                            0x08
    #define SF2_REG_MIB_P0_TXQ0PKT                            0x0C
    #define SF2_REG_MIB_P0_TXBPKTS                            0x10
    #define SF2_REG_MIB_P0_TXMPKTS                            0x14
    #define SF2_REG_MIB_P0_TXUPKTS                            0x18
    #define SF2_REG_MIB_P0_TXCOL                              0x1C
    #define SF2_REG_MIB_P0_TXSINGLECOL                        0x20
    #define SF2_REG_MIB_P0_TXMULTICOL                         0x24
    #define SF2_REG_MIB_P0_TXDEFERREDTX                       0x28
    #define SF2_REG_MIB_P0_TXLATECOL                          0x2C
    #define SF2_REG_MIB_P0_TXEXCESSCOL                        0x30
    #define SF2_REG_MIB_P0_TXFRAMEINDISC                      0x34
    #define SF2_REG_MIB_P0_TXPAUSEPKTS                        0x38
    // SF2 Enhancements
    #define SF2_REG_MIB_P0_TXQ1PKT                            0x3c
    #define SF2_REG_MIB_P0_TXQ2PKT                            0x40
    #define SF2_REG_MIB_P0_TXQ3PKT                            0x44
    #define SF2_REG_MIB_P0_TXQ4PKT                            0x48
    #define SF2_REG_MIB_P0_TXQ5PKT                            0x4c
    // SF2 Done
    #define SF2_REG_MIB_P0_RXOCTETS                           0x50
    #define SF2_REG_MIB_P0_RXUNDERSIZEPKTS                    0x58
    #define SF2_REG_MIB_P0_RXPAUSEPKTS                        0x5c
    #define SF2_REG_MIB_P0_RX64OCTPKTS                        0x60
    #define SF2_REG_MIB_P0_RX127OCTPKTS                       0x64
    #define SF2_REG_MIB_P0_RX255OCTPKTS                       0x68
    #define SF2_REG_MIB_P0_RX511OCTPKTS                       0x6c
    #define SF2_REG_MIB_P0_RX1023OCTPKTS                      0x70
    #define SF2_REG_MIB_P0_RXMAXOCTPKTS                       0x74
    #define SF2_REG_MIB_P0_RXOVERSIZE                         0x78
    #define SF2_REG_MIB_P0_RXJABBERS                          0x7c
    #define SF2_REG_MIB_P0_RXALIGNERRORS                      0x80
    #define SF2_REG_MIB_P0_RXFCSERRORS                        0x84
    #define SF2_REG_MIB_P0_RXGOODOCT                          0x88
    #define SF2_REG_MIB_P0_RXDROPS                            0x90
    #define SF2_REG_MIB_P0_RXUPKTS                            0x94
    #define SF2_REG_MIB_P0_RXMPKTS                            0x98
    #define SF2_REG_MIB_P0_RXBPKTS                            0x9c
    #define SF2_REG_MIB_P0_RXSACHANGES                        0xa0
    #define SF2_REG_MIB_P0_RXFRAGMENTS                        0xa4
    #define SF2_REG_MIB_P0_RXJUMBOPKT                         0xa8
    #define SF2_REG_MIB_P0_RXSYMBOLERRORS                     0xAc
    #define SF2_REG_MIB_P0_RXINRANGEERR                       0xB0
    #define SF2_REG_MIB_P0_RXOUTRANGEERR                      0xB4
    #define SF2_REG_MIB_P0_EEELPIEVEVT                        0xB8
    #define SF2_REG_MIB_P0_EEELPIDURATION                     0xBc
    #define SF2_REG_MIB_P0_RXDISCARD                          0xC0
    #define SF2_REG_MIB_P0_TXQ6PKT                            0xC8
    #define SF2_REG_MIB_P0_TXQ7PKT                            0xCC
    #define SF2_REG_MIB_P0_TX64OCTPKTS                        0xD0
    #define SF2_REG_MIB_P0_TX127OCTPKTS                       0xD4
    #define SF2_REG_MIB_P0_TX255OCTPKTS                       0xD8
    #define SF2_REG_MIB_P0_TX511OCTPKTS                       0xDC
    #define SF2_REG_MIB_P0_TX1023OCTPKTS                      0xE0
    #define SF2_REG_MIB_P0_TXMAXOCTPKTS                       0xE4

/****************************************************************************
   QOS : Page (0x30)
****************************************************************************/
    #define SF2_REG_QOS_GLOBAL_CTRL                           0x00
        #define SF2_QOS_P8_AGGREGATION_MODE                   0x80 /* When set the IMP operated as the uplink port to the upstream network
                                                                      processor and the COS is decided from the TC based the normal packet
                                                                      classification flow. Otherwise, the IMP operates as the interface to
                                                                      the management CPU, and the COS is decided based on the reasons for
                                                                      forwarding the packet to the CPU. */

    #define SF2_REG_PORT_ID_PRIO_MAP                          (0x48+QOS_REG_SHIFT)
    #define SF2_REG_PORTN_TC_SELECT_TABLE                     (0x50+QOS_REG_SHIFT)
        #define SF2_QOS_TC_SRC_SEL_PKT_TYPE_MASK              7
        #define SF2_QOS_TC_SRC_SEL_VAL_MASK                   3
    #define SF2_REG_PORTN_TC_TO_COS                           0x70
        #define SF2_QOS_TC_SRC_SEL_PKT_TYPE_ALL               0x8
        #define SF2_QOS_TC_MAX                                0x7
        #define SF2_QOS_COS_MASK                              0x7
        #define SF2_QOS_COS_SHIFT                             0x3
    #define SF2_REG_QOS_PCP_P7                                (0x28+QOS_REG_SHIFT)
    #define SF2_REG_QOS_PCP_IMP0                              (0x2c+QOS_REG_SHIFT)

/****************************************************************************
    MAC-BASED Port Trunking (LAG - Link Aggregation) : Page (0x32)
****************************************************************************/

    #define REG_IMP0_TRUNK_CTL                            0x02
        #define TRUNK_IMP0_GRP_CTL                        0x1FF
        #define TRUNK_IMP0_GRP_CTL_S                      0
        #define TRUNK_IMP0_GRP_CTL_M                      0x1FF

    #define REG_HASH_WT_TRUNK_CTL                         0x20
        #define HASH_WT_TRUNK_CTL_OVRD                    (1<<31)
        #define HAS_WT_MEM_M                              0xFF
        #define HAS_WT_MEM_0_S                            0
        #define HAS_WT_MEM_1_S                            8
        #define HAS_WT_MEM_2_S                            16

/****************************************************************************
   SF2 Storm Control : Page (0x41)
****************************************************************************/

#define PAGE_PORT_STORM_CONTROL                                0x41

#define REG_PN_STORM_CTL_RATE_PORT_0                           0x10
    #define REG_PN_BUCK0_REF_CNT_M                             0xff
    #define REG_PN_BUCK0_REF_CNT_S                             0
    #define REG_PN_BUCK0_SIZE_M                                0x7
    #define REG_PN_BUCK0_SIZE_S                                8
    #define REG_PN_BUCK1_REF_CNT_M                             0xff
    #define REG_PN_BUCK1_REF_CNT_S                             11
    #define REG_PN_BUCK1_SIZE_M                                0x7
    #define REG_PN_BUCK1_SIZE_S                                19
    #define REG_PN_BUCK0_ENABLE_MASK                           0x400000
    #define REG_PN_BUCK1_ENABLE_MASK                           0x800000
    #define REG_PN_BUCK0_MODE_MASK                             0x20000000
    #define REG_PN_BUCK1_MODE_MASK                             0x40000000

#define REG_PN_STORM_CTL_CFG_PORT_0                            0x34
    #define REG_PN_BUCK0_PKT_SEL_M                             0x7f
    #define REG_PN_BUCK0_PKT_SEL_S                             0x0
    #define REG_PN_BUCK1_PKT_SEL_M                             0x7f
    #define REG_PN_BUCK1_PKT_SEL_S                             0x8
        #define UNICAST_LOOKUP_HIT                             0x1
        #define MULTICAST_LOOKUP_HIT                           0x2
        #define RESERVED_MAC_ADDRESS                           0x4
        #define BROADCAST                                      0x8
        #define MULTICAST_LOOKUP_FAIL                          0x10
        #define UNICAST_LOOKUP_FAIL                            0x20
        #define RESERVED                                       0x40
    #define REG_PN_BUCK0_IFG_BYTES_MASK                        0x80
    #define REG_PN_BUCK1_PKT_SEL_MASK                          0x7f00
    #define REG_PN_BUCK1_IFG_BYTES_MASK                        0x8000

/****************************************************************************
   QOS Scheduler : Page (0x46)
****************************************************************************/

#define PAGE_QOS_SCHEDULER                                     0x46

// default is Strict Priority on all q's of all ports.
#define REG_PN_QOS_PRI_CTL_PORT_0                              0x0
    #define REG_PN_QOS_PRI_CTL_SZ                              0x1   // 1 Byte per port
    #define  PN_QOS_SCHED_SEL_M                                0x7
    #define  PN_QOS_SCHED_SEL_S                                0

    #define SF2_ALL_Q_SP                                       0
    #define SF2_Q7_SP                                          1
    #define SF2_Q7_Q6_SP                                       2
    #define SF2_Q7_Q5_SP                                       3
    #define SF2_Q7_Q4_SP                                       4
    #define SF2_ALL_Q_WRR                                      5
  /* Granularity is 1 packet or 256 bytes */
    #define  PN_QOS_WDRR_GRAN_M                                0x1
    #define  PN_QOS_WDRR_GRAN_S                                3

    #define SF2_WRR_PKT                                        1
    #define SF2_WDRR_PKT                                       0

#define REG_PN_QOS_WEIGHT_PORT_0                               0x10
                                             // 8q's x 1Byte per port
                                             // [q7.. q0]
#define REG_PN_QOS_WEIGHTS                                     0x8

/****************************************************************************
   Egress Shaper control : Page (0x47)
****************************************************************************/

#define PAGE_PORT_EGRESS_SHAPER                                0x47

    #define SF2_REG_PN_SHAPER_RATE_BYTE                        0x0
    #define SF2_REG_P7_SHAPER_RATE_BYTE                        0x1C
    #define SF2_REG_P8_SHAPER_RATE_BYTE                        0x20

    #define SF2_REG_PN_SHAPER_BURST_SZ_BYTE                    0x30
    #define SF2_REG_P7_SHAPER_BURST_SZ_BYTE                    0x4C
    #define SF2_REG_P8_SHAPER_BURST_SZ_BYTE                    0x50

    #define SF2_REG_PN_SHAPER_STAT                             0x60
    #define SF2_REG_P7_SHAPER_STAT                             0x7C
    #define SF2_REG_P8_SHAPER_STAT                             0x80

    #define SF2_REG_PN_SHAPER_RATE_PKT                         0x90
    #define SF2_REG_P7_SHAPER_RATE_PKT                         0xAC
    #define SF2_REG_P8_SHAPER_RATE_PKT                         0xB0

    #define SF2_REG_PN_SHAPER_BURST_SZ_PKT                     0xC0
    #define SF2_REG_P7_SHAPER_BURST_SZ_PKT                     0xDC
    #define SF2_REG_P8_SHAPER_BURST_SZ_PKT                     0xE0

        #define SHAPER_RATE_BURST_VAL_MASK                     0x3FFFFU
        #define SHAPER_PACKET_MODE                             1
        #define SHAPER_STAT_COUNT_MASK                         0xFFFFFFFU
        #define SHAPER_STAT_OVF_MASK                           0x10000000U  // Overflow mask
        #define SHAPER_STAT_INPF_MASK                          0x10000000U  // In Profile mask

    #define SF2_REG_SHAPER_ENB_AVB                             0xE4
    #define SF2_REG_SHAPER_ENB                                 0xE6
    #define SF2_REG_SHAPER_ENB_PKT_BASED                       0xE8
    #define SF2_REG_SHAPER_BLK_CTRL_ENB                        0xEA
    #define SF2_REG_SHAPER_INC_IFG_CTRL                        0xEC   // On port shaper only

/****************************************************************************
   Egress Per QUEUE Shaper control : Page (0x48)
****************************************************************************/

#define PAGE_Q0_EGRESS_SHAPER                                0x48
#define PAGE_Q1_EGRESS_SHAPER                                0x49
#define PAGE_Q2_EGRESS_SHAPER                                0x4A
#define PAGE_Q3_EGRESS_SHAPER                                0x4B
#define PAGE_Q4_EGRESS_SHAPER                                0x4C
#define PAGE_Q5_EGRESS_SHAPER                                0x4D
#define PAGE_Q6_EGRESS_SHAPER                                0x4E
#define PAGE_Q7_EGRESS_SHAPER                                0x4F



#endif /* _BCMMII_XTN_H_ */
