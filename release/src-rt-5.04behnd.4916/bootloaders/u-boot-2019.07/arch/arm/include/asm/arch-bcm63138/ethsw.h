/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 Broadcom Ltd.
 */

#ifndef _63138_ETHSW_H
#define _63138_ETHSW_H

#define SWITCH_BASE              0x80080000

typedef struct {
    uint32_t led_f;
    uint32_t reserved;
} LED_F;

typedef struct EthernetSwitchCore
{
    uint32_t port_traffic_ctrl[9];            /* 0x00 - 0x08 */
    uint32_t reserved1[2];                    /* 0x09 - 0x0a */
    uint32_t switch_mode;                     /* 0x0b */
#define ETHSW_SM_RETRY_LIMIT_DIS                  0x04
#define ETHSW_SM_FORWARDING_EN                    0x02
#define ETHSW_SM_MANAGED_MODE                     0x01
    uint32_t pause_quanta;                    /* 0x0c */
    uint32_t reserved33; 
    uint32_t imp_port_state;                  /*0x0e */
#define ETHSW_IPS_USE_MII_HW_STS                  0x00
#define ETHSW_IPS_USE_REG_CONTENTS                0x80
#define ETHSW_IPS_GMII_SPEED_UP_NORMAL            0x00
#define ETHSW_IPS_GMII_SPEED_UP_2G                0x40
#define ETHSW_IPS_TXFLOW_NOT_PAUSE_CAPABLE        0x00
#define ETHSW_IPS_TXFLOW_PAUSE_CAPABLE            0x20
#define ETHSW_IPS_RXFLOW_NOT_PAUSE_CAPABLE        0x00
#define ETHSW_IPS_RXFLOW_PAUSE_CAPABLE            0x10
#define ETHSW_IPS_SW_PORT_SPEED_1000M_2000M       0x08
#define ETHSW_IPS_DUPLEX_MODE                     0x02
#define ETHSW_IPS_LINK_FAIL                       0x00
#define ETHSW_IPS_LINK_PASS                       0x01
    uint32_t led_refresh;                     /* 0x0f */
    LED_F        led_function[2];                 /* 0x10 */
    uint32_t led_function_map;                /* 0x14 */
    uint32_t reserved14; 
    uint32_t led_enable_map;                  /* 0x16 */
    uint32_t reserved15; 
    uint32_t led_mode_map0;                   /* 0x18 */
    uint32_t reserved16; 
    uint32_t led_function_map1;               /* 0x1a */
    uint32_t reserved17; 
    uint32_t reserved2[5];                    /* 0x1c - 0x20 */
    uint32_t port_forward_ctrl;               /* 0x21 */
    uint32_t switch_ctrl;                     /* 0x22 */
#define ETHSW_SC_MII_DUMP_FORWARDING_EN           0x40
#define ETHSW_SC_MII2_VOL_SEL                     0x02
    uint32_t reserved3;                       /* 0x23 */
    uint32_t protected_port_selection;        /* 0x24 */
    uint32_t reserved18; 
    uint32_t wan_port_select;                 /* 0x26 */
    uint32_t reserved19; 
    uint32_t pause_capability;                /* 0x28 */
    uint32_t reserved20[3]; 
    uint32_t reserved4[3];                    /* 0x2c - 0x2e */
    uint32_t reserved_multicast_control;      /* 0x2f */
    uint32_t reserved5;                       /* 0x30 */
    uint32_t txq_flush_mode_control;          /* 0x31 */
    uint32_t ulf_forward_map;                 /* 0x32 */
    uint32_t reserved21; 
    uint32_t mlf_forward_map;                 /* 0x34 */
    uint32_t reserved22; 
    uint32_t mlf_impc_forward_map;            /* 0x36 */
    uint32_t reserved23; 
    uint32_t pause_pass_through_for_rx;       /* 0x38 */
    uint32_t reserved24; 
    uint32_t pause_pass_through_for_tx;       /* 0x3a */
    uint32_t reserved25; 
    uint32_t disable_learning;                /* 0x3c */
    uint32_t reserved26; 
    uint32_t reserved6[26];                   /* 0x3e - 0x57 */
    uint32_t port_state_override[8];          /* 0x58 - 0x5f */
#define ETHSW_PS_SW_OVERRIDE                      0x40
#define ETHSW_PS_SW_TX_FLOW_CTRL_EN               0x20
#define ETHSW_PS_SW_RX_FLOW_CTRL_EN               0x10
#define ETHSW_PS_SW_PORT_SPEED_1000M              0x80
#define ETHSW_PS_SW_PORT_SPEED_100M               0x40
#define ETHSW_PS_SW_PORT_SPEED_10M                0x00
#define ETHSW_PS_DUPLEX_MODE                      0x02
#define ETHSW_PS_LINK_DOWN                        0x00
#define ETHSW_PS_LINK_UP                          0x01
    uint32_t reserved7[4];                    /* 0x60 - 0x63 */
    uint32_t imp_rgmii_ctrl_p4;               /* 0x64 */
    uint32_t imp_rgmii_ctrl_p5;               /* 0x65 */
    uint32_t reserved8[6];                    /* 0x66 - 0x6b */
    uint32_t rgmii_timing_delay_p4;           /* 0x6c */
    uint32_t gmii_timing_delay_p5;            /* 0x6d */
    uint32_t reserved9[11];                   /* 0x6e - 0x78 */
    uint32_t software_reset;                  /* 0x79 */
    uint32_t reserved13[6];                   /* 0x7a - 0x7f */
    uint32_t pause_frame_detection;           /* 0x80 */
    uint32_t reserved10[7];                   /* 0x81 - 0x87 */
    uint32_t fast_aging_ctrl;                 /* 0x88 */
    uint32_t fast_aging_port;                 /* 0x89 */
    uint32_t fast_aging_vid;                  /* 0x8a */
    uint32_t anonymous1[376];                 /* 0x8b */
    uint32_t brcm_hdr_ctrl;                   /* 0x203 */
    uint32_t anonymous2[0x2efc];              /* 0x204 */
    uint32_t port_vlan_ctrl[9*2];               /* 0x3100 */
} EthernetSwitchCore;

#define PBMAP_MIPS 0x100

typedef struct EthernetSwitchReg
{
    uint32_t switch_ctrl;                      /* 0x0000 */
    uint32_t switch_status;                    /* 0x0004 */
    uint32_t dir_data_write_reg;               /* 0x0008 */
    uint32_t dir_data_read_reg;                /* 0x000c */
    uint32_t led_serial_refresh_time_unit;     /* 0x0010 */
    uint32_t reserved1;                        /* 0x0014 */
    uint32_t switch_rev;                       /* 0x0018 */
    uint32_t phy_rev;                          /* 0x001c */
    uint32_t phy_test_ctrl;                    /* 0x0020 */
    uint32_t qphy_ctrl;                        /* 0x0024 */
#define ETHSW_QPHY_CTRL_PHYAD_BASE_SHIFT     12
#define ETHSW_QPHY_CTRL_PHYAD_BASE_MASK      (0x1f<<ETHSW_QPHY_CTRL_PHYAD_BASE_SHIFT)
#define ETHSW_QPHY_CTRL_RESET_SHIFT          8
#define ETHSW_QPHY_CTRL_RESET_MASK           (0x1<<ETHSW_QPHY_CTRL_RESET_SHIFT )
#define ETHSW_QPHY_CTRL_CK25_DIS_SHIFT       7
#define ETHSW_QPHY_CTRL_CK25_DIS_MASK        (0x1<<ETHSW_QPHY_CTRL_CK25_DIS_SHIFT)
#define ETHSW_QPHY_CTRL_EXT_PWR_DOWN_SHIFT   1
#define ETHSW_QPHY_CTRL_EXT_PWR_DOWN_MASK    (0xf<<ETHSW_QPHY_CTRL_EXT_PWR_DOWN_SHIFT)
#define ETHSW_QPHY_CTRL_IDDQ_BIAS_SHIFT      0
#define ETHSW_QPHY_CTRL_IDDQ_BIAS_MASK       (0x1<<ETHSW_QPHY_CTRL_IDDQ_BIAS_SHIFT)
    uint32_t qphy_status;                      /* 0x0028 */
    uint32_t sphy_ctrl;                        /* 0x002c */
#define ETHSW_SPHY_CTRL_PHYAD_SHIFT          8
#define ETHSW_SPHY_CTRL_PHYAD_MASK           (0x1f<<ETHSW_SPHY_CTRL_PHYAD_SHIFT)
#define ETHSW_SPHY_CTRL_RESET_SHIFT          5
#define ETHSW_SPHY_CTRL_RESET_MASK           (0x1<<ETHSW_SPHY_CTRL_RESET_SHIFT )
#define ETHSW_SPHY_CTRL_CK25_DIS_SHIFT       4
#define ETHSW_SPHY_CTRL_CK25_DIS_MASK        (0x1<<ETHSW_SPHY_CTRL_CK25_DIS_SHIFT)
#define ETHSW_SPHY_CTRL_EXT_PWR_DOWN_SHIFT   1
#define ETHSW_SPHY_CTRL_EXT_PWR_DOWN_MASK    (0x1<<ETHSW_SPHY_CTRL_EXT_PWR_DOWN_SHIFT)
#define ETHSW_SPHY_CTRL_IDDQ_BIAS_SHIFT      0
#define ETHSW_SPHY_CTRL_IDDQ_BIAS_MASK       (0x1<<ETHSW_SPHY_CTRL_IDDQ_BIAS_SHIFT)
    uint32_t sphy_status;                      /* 0x0030 */
    uint32_t reserved2[15];                    /* 0x0034 */
    uint32_t rgmii_5_ctrl;                     /* 0x0070 */
    uint32_t rgmii_5_ib_status;                /* 0x0074 */
    uint32_t rgmii_5_rx_clk_delay_ctrl;        /* 0x0078 */
    uint32_t rgmii_7_ctrl;                     /* 0x007c */
    uint32_t rgmii_7_ib_status;                /* 0x0080 */
    uint32_t rgmii_7_rx_clk_delay_ctrl;        /* 0x0084 */
    uint32_t led_blink_rate_ctrl;              /* 0x0088 */
    uint32_t led_serial_ctrl;                  /* 0x008c */
    uint32_t led_ctrl[5];                      /* 0x0090 */
#define ETHSW_LED_CTRL_SPD0_ON               0x0
#define ETHSW_LED_CTRL_SPD0_OFF              0x1
#define ETHSW_LED_CTRL_SPD1_ON               0x0
#define ETHSW_LED_CTRL_SPD1_OFF              0x2
#define ETHSW_LED_CTRL_1000M_SHIFT           6
#define ETHSW_LED_CTRL_100M_SHIFT            4
#define ETHSW_LED_CTRL_10M_SHIFT             2
#define ETHSW_LED_CTRL_NOLINK_SHIFT          0
#define ETHSW_LED_CTRL_ALL_SPEED_MASK        0xff
#define ETHSW_LED_CTRL_SPEED_MASK            0x3
    uint32_t reserved3[2];                     /* 0x00a4 */
    uint32_t crossbar_switch_ctrl;             /* 0x00ac */
    uint32_t reserved4[6];                     /* 0x00b0 */
    uint32_t rgmii_11_ctrl;                    /* 0x00c8 */
    uint32_t rgmii_11_ib_status;               /* 0x00cc */
    uint32_t rgmii_11_rx_clk_delay_ctrl;       /* 0x00d0 */
    uint32_t rgmii_12_ctrl;                    /* 0x00d4 */
#define ETHSW_RC_MII_MODE_MASK               0x1c
#define ETHSW_RC_EXT_RVMII                   0x10
#define ETHSW_RC_EXT_GPHY                    0x0c
#define ETHSW_RC_EXT_EPHY                    0x08
#define ETHSW_RC_INT_GPHY                    0x04
#define ETHSW_RC_INT_EPHY                    0x00
#define ETHSW_RC_ID_MODE_DIS                 0x02
#define ETHSW_RC_RGMII_EN                    0x01
    uint32_t rgmii_12_ib_status;               /* 0x00d8 */
    uint32_t rgmii_12_rx_clk_delay_ctrl;       /* 0x00dc */
#define ETHSW_RXCLK_IDDQ                     (1<<4)
#define ETHSW_RXCLK_BYPASS                   (1<<5)
    uint32_t anonymous1[44];                   /* 0x00e0 */
    uint32_t single_serdes_rev;                /* 0x0190 */ /* From here for B0 and late chip only */
    uint32_t single_serdes_ctrl;               /* 0x0194 */
    uint32_t single_serdes_stat;               /* 0x0198 */
    uint32_t led_wan_ctrl;                     /* 0x019c */
} EthernetSwitchReg;

#endif
