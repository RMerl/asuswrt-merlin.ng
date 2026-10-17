#include <config.h>
#include <common.h>
#include <stdlib.h>

#define uint32 uint32_t
/*
** Eth Switch Registers
*/
typedef struct {
    unsigned int led_f;
    unsigned int reserved;
} LED_F;

typedef struct EthernetSwitchCore
{
    unsigned int port_traffic_ctrl[9];            /* 0x00 - 0x08 */
    unsigned int reserved1[2];                    /* 0x09 - 0x0a */
    unsigned int switch_mode;                     /* 0x0b */
#define ETHSW_SM_RETRY_LIMIT_DIS                  0x04
#define ETHSW_SM_FORWARDING_EN                    0x02
#define ETHSW_SM_MANAGED_MODE                     0x01
    unsigned int pause_quanta;                    /* 0x0c */
    unsigned int reserved33; 
    unsigned int imp_port_state;                  /*0x0e */
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
    unsigned int led_refresh;                     /* 0x0f */
    LED_F        led_function[2];                 /* 0x10 */
    unsigned int led_function_map;                /* 0x14 */
    unsigned int reserved14; 
    unsigned int led_enable_map;                  /* 0x16 */
    unsigned int reserved15; 
    unsigned int led_mode_map0;                   /* 0x18 */
    unsigned int reserved16; 
    unsigned int led_function_map1;               /* 0x1a */
    unsigned int reserved17; 
    unsigned int reserved2[5];                    /* 0x1c - 0x20 */
    unsigned int port_forward_ctrl;               /* 0x21 */
    unsigned int switch_ctrl;                     /* 0x22 */
#define ETHSW_SC_MII_DUMP_FORWARDING_EN           0x40
#define ETHSW_SC_MII2_VOL_SEL                     0x02
    unsigned int reserved3;                       /* 0x23 */
    unsigned int protected_port_selection;        /* 0x24 */
    unsigned int reserved18; 
    unsigned int wan_port_select;                 /* 0x26 */
    unsigned int reserved19; 
    unsigned int pause_capability;                /* 0x28 */
    unsigned int reserved20[3]; 
    unsigned int reserved4[3];                    /* 0x2c - 0x2e */
    unsigned int reserved_multicast_control;      /* 0x2f */
    unsigned int reserved5;                       /* 0x30 */
    unsigned int txq_flush_mode_control;          /* 0x31 */
    unsigned int ulf_forward_map;                 /* 0x32 */
    unsigned int reserved21; 
    unsigned int mlf_forward_map;                 /* 0x34 */
    unsigned int reserved22; 
    unsigned int mlf_impc_forward_map;            /* 0x36 */
    unsigned int reserved23; 
    unsigned int pause_pass_through_for_rx;       /* 0x38 */
    unsigned int reserved24; 
    unsigned int pause_pass_through_for_tx;       /* 0x3a */
    unsigned int reserved25; 
    unsigned int disable_learning;                /* 0x3c */
    unsigned int reserved26; 
    unsigned int reserved6[26];                   /* 0x3e - 0x57 */
    unsigned int port_state_override[8];          /* 0x58 - 0x5f */
#define ETHSW_PS_SW_OVERRIDE                      0x40
#define ETHSW_PS_SW_TX_FLOW_CTRL_EN               0x20
#define ETHSW_PS_SW_RX_FLOW_CTRL_EN               0x10
#define ETHSW_PS_SW_PORT_SPEED_1000M              0x08
#define ETHSW_PS_SW_PORT_SPEED_100M               0x04
#define ETHSW_PS_SW_PORT_SPEED_10M                0x00
#define ETHSW_PS_DUPLEX_MODE                      0x02
#define ETHSW_PS_LINK_DOWN                        0x00
#define ETHSW_PS_LINK_UP                          0x01
    unsigned int reserved7[4];                    /* 0x60 - 0x63 */
    unsigned int imp_rgmii_ctrl_p4;               /* 0x64 */
    unsigned int imp_rgmii_ctrl_p5;               /* 0x65 */
    unsigned int reserved8[6];                    /* 0x66 - 0x6b */
    unsigned int rgmii_timing_delay_p4;           /* 0x6c */
    unsigned int gmii_timing_delay_p5;            /* 0x6d */
    unsigned int reserved9[11];                   /* 0x6e - 0x78 */
    unsigned int software_reset;                  /* 0x79 */
    unsigned int reserved13[6];                   /* 0x7a - 0x7f */
    unsigned int pause_frame_detection;           /* 0x80 */
    unsigned int reserved10[7];                   /* 0x81 - 0x87 */
    unsigned int fast_aging_ctrl;                 /* 0x88 */
    unsigned int fast_aging_port;                 /* 0x89 */
    unsigned int fast_aging_vid;                  /* 0x8a */
    unsigned int anonymous1[376];                 /* 0x8b */
    unsigned int brcm_hdr_ctrl;                   /* 0x203 */
    unsigned int anonymous2[0x2efc];              /* 0x204 */
    unsigned int port_vlan_ctrl[9*2];               /* 0x3100 */
} EthernetSwitchCore;

#define PBMAP_MIPS 0x100

typedef struct {
    uint32 led_ctrl;
    uint32 led_encoding_sel;
    uint32 led_encoding;
}LED_CFG;

typedef struct EthernetSwitchReg
{
    uint32 switch_ctrl;                      /* 0x0000 */
    uint32 switch_status;                    /* 0x0004 */
    uint32 dir_data_write_reg;               /* 0x0008 */
    uint32 dir_data_read_reg;                /* 0x000c */
    uint32 switch_rev;                       /* 0x0010 */
    uint32 phy_rev;                          /* 0x0014 */
    uint32 phy_test_ctrl;                    /* 0x0018 */
    uint32 qphy_ctrl;                        /* 0x001c */
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
#define ETHSW_QPHY_CTRL_IDDQ_GLOBAL_PWR_SHIFT      6
#define ETHSW_QPHY_CTRL_IDDQ_GLOBAL_PWR_MASK       (0x1<<ETHSW_QPHY_CTRL_IDDQ_GLOBAL_PWR_SHIFT)


    uint32 qphy_status;                      /* 0x0020 */
    uint32 sphy_ctrl;                        /* 0x0024 */
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
#define ETHSW_SPHY_CTRL_IDDQ_GLOBAL_PWR_SHIFT      3
#define ETHSW_SPHY_CTRL_IDDQ_GLOBAL_PWR_MASK       (0x1<<ETHSW_SPHY_CTRL_IDDQ_GLOBAL_PWR_SHIFT)
    uint32 sphy_status;                      /* 0x0028 */
    uint32 switch_phy_intr_ctrl;             /* 0x002c */
    uint32 moca_bp_qsel_ctrl;                /* 0x0030 */
    uint32 reserved2[3];                     /* 0x0034 - 0x003f */
    LED_CFG led_ctrl[8];                     /* 0x0040 - 0x009f */
    LED_CFG led_wan_ctrl;                    /* 0x00a0 - 0x00ab */
#define ETHSW_LED_CTRL_SPD0_ON               0x0
#define ETHSW_LED_CTRL_SPD0_OFF              0x1
#define ETHSW_LED_CTRL_SPD1_ON               0x0
#define ETHSW_LED_CTRL_SPD1_OFF              0x2
#define ETHSW_LED_CTRL_1000M_SHIFT           9
#define ETHSW_LED_CTRL_100M_SHIFT            6
#define ETHSW_LED_CTRL_10M_SHIFT             3
#define ETHSW_LED_CTRL_NOLINK_SHIFT          0
#define ETHSW_LED_CTRL_ALL_SPEED_MASK        0x3ffff
#define ETHSW_LED_CTRL_SPEED_MASK            0x7
    uint32 led_blink_rate_ctrl;              /* 0x00ac */
    uint32 led_serial_ctrl;                  /* 0x00b0 */
    uint32 led_refresh_period_ctrl;          /* 0x00b4 */
    uint32 aggregate_led_ctrl;               /* 0x00b8 */
#define ETHSW_AGGREGATE_LED_CTRL_PORT_EN_MASK            0xffff
#define ETHSW_AGGREGATE_LED_CTRL_ACT_SEL_MASK            0x10000
#define ETHSW_AGGREGATE_LED_CTRL_ACT_POL_SEL_MASK        0x20000
#define ETHSW_AGGREGATE_LED_CTRL_LNK_POL_SEL_MASK        0x40000
    uint32 aggregate_blink_rate_ctrl;        /* 0x00bc */
    uint32 reserved3[2];                     /* 0x00c0 - 0x00c7 */
    uint32 crossbar_switch_ctrl;             /* 0x00c8 */
    uint32 reserved4[32];                    /* 0x00cc - 0x014b */
    uint32 rgmii_11_ctrl;                    /* 0x014c */
#define ETHSW_RC_MII_MODE_MASK               0x1c
#define ETHSW_RC_EXT_RVMII                   0x10
#define ETHSW_RC_EXT_GPHY                    0x0c
#define ETHSW_RC_EXT_EPHY                    0x08
#define ETHSW_RC_INT_GPHY                    0x04
#define ETHSW_RC_INT_EPHY                    0x00
#define ETHSW_RC_ID_MODE_DIS                 0x02
#define ETHSW_RC_RGMII_EN                    0x01
    uint32 rgmii_11_ib_status;               /* 0x0150 */
    uint32 rgmii_11_rx_clk_delay_ctrl;       /* 0x0154 */
#define ETHSW_RXCLK_IDDQ                    (1<<4)
#define ETHSW_RXCLK_BYPASS                  (1<<5)
    uint32 anonymous1[154];                  /* 0x0158 - 0x03bf*/
    uint32 single_serdes_rev;                /* 0x03c0 */
    uint32 single_serdes_ctrl;               /* 0x03c4 */
    uint32 single_serdes_stat;               /* 0x03c8 */
    uint32 single_serdes_apd_ctrl;           /* 0x03cc */
    uint32 single_serdes_apd_fsm_ctrl;       /* 0x03d0 */
} EthernetSwitchReg;


