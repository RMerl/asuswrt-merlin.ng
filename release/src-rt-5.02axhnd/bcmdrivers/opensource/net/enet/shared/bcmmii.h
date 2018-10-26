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
#ifndef _BCMMII_H_
#define _BCMMII_H_

#include "bcmmii_exports.h"

/****************************************************************************
    Broadcom Extended PHY registers
****************************************************************************/
#define BRCM_MIIEXT_BANK            0x1f
    #define BRCM_MIIEXT_BANK_MASK       0xfff0
    #define BRCM_MIIEXT_ADDR_RANGE      0xffe0
    #define BRCM_MIIEXT_DEF_BANK        0x8000
#define BRCM_MIIEXT_OFFSET          0x10
    #define BRCM_MIIEXT_OFF_MASK    0xf

#define MIIEX_DIGITAL_STATUS_1000X          0x8304
    #define MIIEX_SPEED_SHIFT       3
    #define MIIEX_SPEED             (3<<MIIEX_SPEED_SHIFT)
        #define MIIEX_SPD10         (0<<MIIEX_SPEED_SHIFT)
        #define MIIEX_SPD100        (1<<MIIEX_SPEED_SHIFT)
        #define MIIEX_SPD1000       (2<<MIIEX_SPEED_SHIFT)
        #define MIIEX_SPD2500       (3<<MIIEX_SPEED_SHIFT)
    #define MIIEX_DUPLEX            (1<<2)
    #define MIIEX_LINK              (1<<1)
    #define MIIEX_SGMII_MODE        (1<<0)

#define PLL_AFE_CTRL1               0x8051
    #define PLL_AFE_PLL_PON_SHIFT   6
    #define PLL_AFE_PLL_PON_MASK    (0xf << PLL_AFE_PLL_PON_SHIFT)

#define TX_AFE_CTRL2                0x8067
    #define TX_AFE_TX_PON_SHIFT     3
    #define TX_AFE_TX_PON_MASK      (0xf << TX_AFE_TX_PON_SHIFT)

#define RX_AFE_CTRL0                0x80b2
    #define RX_AFE_RX_PON_SHIFT     8
    #define RX_AFE_RX_PON_MASK      (0xf << RX_AFE_RX_PON_SHIFT)

#define RX_AFE_CTRL2                0x80b5
    #define RX_AFE_CTRL2_DIV4       0x2
    #define RX_AFE_CTRL2_DIV10      0x20

/****************************************************************************
    Internal PHY registers
****************************************************************************/

//#define BCM_PHY_ID_M                        0x1F
//#define IsExtPhyId(id)                      ((id & BCM_PHY_ID_M) >= 0x10)
#define BCM_EXT_6829                        0x80
#define IsExt6829(id)                       (((id) & BCM_EXT_6829) && ((id & 0xFF) != 0xFF))
//#define isPhyConnected(phy_id)              (phy_id != 0xFF)

#define MII_CONTROL                         0x00
#define CL45_REG_IEEE_CTRL                  0x10000
    #define MII_CONTROL_RESET               (1 << 15)
    #define MII_CONTROL_LOOPBACK            (1 << 14)
    #define MII_CONTROL_SPEED_SEL13         (1 << 13)
    #define MII_CONTROL_AN_ENABLE           (1 << 12)

    #define MII_CONTROL_POWER_DOWN          (1 << 11)
    #define MII_CONTROL_ISOLATE_MII         (1 << 10)
    #define MII_CONTROL_RESTART_AUTONEG     (1 << 9)
    #define MII_CONTROL_DUPLEX_MODE         (1 << 8)

    #define MII_CONTROL_COLLISION_TEST      (1 << 7)
    #define MII_CONTROL_SPEED_SEL6          (1 << 6)

    /* Note: The IEEE fixed speed configurations below do not work for 8486x and 8488x PHYs,
        need to use AN with speed advertisement to accomplish the same purpose for those PHYs */
    #define MII_CONTROL_SPEED_MASK  (MII_CONTROL_SPEED_SEL13|MII_CONTROL_SPEED_SEL6)
    #define MII_CONTROL_SPEED_b5_2  (1<<13|1<<6)
    #define MII_CONTROL_SPEED_1000M (0<<13|1<<6)
    #define MII_CONTROL_SPEED_100M  (1<<13|0<<6)
    #define MII_CONTROL_SPEED_10M   (0)

    #define MII_CONTROL_SPEED_MASK_b5_2     (0xf<<2)
    #define MII_CONTROL_SPEED_5G            (7<<2)
    #define MII_CONTROL_SPEED_2P5G          (6<<2)
    #define MII_CONTROL_SPEED_400G          (5<<2)
    #define MII_CONTROL_SPEED_25G           (4<<2)
    #define MII_CONTROL_SPEED_100G          (3<<2)
    #define MII_CONTROL_SPEED_40G           (2<<2)
    #define MII_CONTROL_SPEED_10PASS        (1<<2)
    #define MII_CONTROL_SPEED_10G           (0<<2)

#define MII_STATUS                          0x1
    #define MII_STATUS_LINK                 (1<<2)

#define MII_DEVID1                          0x2
#define MII_DEVID2                          0x3
#define CL45_REG(CL22_REG)                  (CL45_REG_IEEE_CTRL + CL22_REG)

#define MII_STATUS                          0x1
#define MII_ASR                             0x19
#define MII_INTERRUPT                       0x1a
#define MII_INTERRUPT_MASK                  0x1b
#define MII_AS2SR                           0x1b  // Aux Status 2 Shadow Register (53101)
    #define MII_AS2SR_APD_EN                (1 << 5)
#define MII_REGISTER_1C                     0x1c
    #define MII_1C_WRITE_ENABLE             (1 << 15)
    #define MII_1C_RESERVED_CTRL3_SV        (0x05 << 10)
        #define MII_1C_RESERVED_CTRL3_DFLT  0x001e
        #define MII_1C_AUTO_PWRDN_DLL_DIS   (1 << 1)
        #define MII_1C_CLK125_OUTPUT_DIS    (1 << 0)
    #define MII_1C_AUTO_POWER_DOWN_SV       (0x0a << 10)
        #define MII_1C_APD_COMPATIBILITY    (1 << 8)
        #define MII_1C_AUTO_POWER_DOWN      (1 << 5)
        #define MII_1C_SLEEP_TIMER_SEL      (1 << 4)
        #define MII_1C_WAKEUP_TIMER_SEL_84  (1 << 0)
#define MII_1C_SHADOW_REG_SEL_S         10
#define MII_1C_SHADOW_REG_SEL_M         0x1F
#define MII_1C_SHADOW_CLK_ALIGN_CTRL        0x3
    #define GTXCLK_DELAY_BYPASS_DISABLE     (1 << 9)
#define MII_1C_SHADOW_LED_CONTROL           0x9
    #define ACT_LINK_LED_ENABLE             (1 << 4)
#define MII_1C_EXTERNAL_CONTROL_1           0xB
    #define LOM_LED_MODE                    (1 << 2)
#define MII_BRCM_TEST                       0x1f
    #define MII_BRCM_TEST_SHADOW_ENABLE     0x0080
    #define MII_BRCM_TEST_SHADOW2_ENABLE    0x0004

/* Shadow register 0x18 access */
#define MII_REGISTER_18                     0x18
   #define MII_REG_18_SEL(_s)               (((_s) << 12) | 0x7)
   #define MII_REG_18_WR(_s,_v)             (((_s) == 7 ? 0x8000 : 0) | (_v) | (_s)) /* Shadow 111 bit[15] = 1 for write */      
      #define RGMII_RXD_TO_RXC_SKEW         (1 << 8) /* Based on 54616 but should be good for others */
#define MII_REG_18_SHADOW_MISC_CTRL         0x7


/* MII Auxiliary Status Register */
/*
 * 0x8000 auto negotiation done
 * 0x0004 link up
 * 0x0700 1000 Mbps, FDX
 * 0x0600 1000 Mbps, HDX
 * 0x0500  100 Mbps, FDX
 * 0x0300  100 Mbps, HDX
 * 0x0200   10 Mpbs, FDX
 * 0x0100   10 Mbps, HDX
 */

#define MII_ASR_ANHCD_MASK 0x0700
#define MII_ASR_DONE(r) ((r & 0x8000) != 0)
#define MII_ASR_LINK(r) ((r & 0x0004) != 0)
#define MII_ASR_FDX(r)  (((r & 0x0700) == 0x0700) || ((r & 0x0700) == 0x0500) || ((r & 0x0700) == 0x0200))
#define MII_ASR_1000(r) (((r & 0x0700) == 0x0700) || ((r & 0x0700) == 0x0600))
#define MII_ASR_100(r)  (((r & 0x0700) == 0x0500) || ((r & 0x0700) == 0x0300))
#define MII_ASR_10(r)   (((r & 0x0700) == 0x0200) || ((r & 0x0700) == 0x0100))
#define MII_ASR_NOHCD(r) ((r & MII_ASR_ANHCD_MASK) == 0)

typedef struct {
    unsigned int cfgAng:1,
                 cfgSpd2500Fdx:1,
                 cfgSpd2500Hdx:1,
                 cfgSpd1000Fdx:1,
                 cfgSpd1000Hdx:1,
                 cfgSpd100Fdx:1,
                 cfgSpd100Hdx:1,
                 cfgSpd10Fdx:1,
                 cfgSpd10Hdx:1,
                 lnk:1,
                 fdx:1,
                 spd2500:1,
                 spd1000:1,
                 spd100:1,
                 spd10:1;
} PHY_STAT;

/* MII Interrupt register. */
#define MII_INTR_ENABLE                     0x4000
#define MII_INTR_FDX                        0x0008
#define MII_INTR_SPD                        0x0004
#define MII_INTR_LNK                        0x0002

/***************************************************
 ************* Star Fighter 2 specific  ************
 ***************************************************/

/****************************************************************************
    External switch pseudo PHY: Page (0x00)
****************************************************************************/

#define PAGE_CONTROL                                      0x00

    #define REG_PORT_CTRL                                 0x00

        #define REG_PORT_NO_SPANNING_TREE                 0x00
        #define REG_PORT_STP_STATE_DISABLED               0x20
        #define REG_PORT_STP_STATE_BLOCKING               0x40
        #define REG_PORT_STP_STATE_LISTENING              0x60
        #define REG_PORT_STP_STATE_LEARNING               0x80
        #define REG_PORT_STP_STATE_FORWARDING             0xA0
        #define REG_PORT_TX_DISABLE                       0x02
        #define REG_PORT_RX_DISABLE                       0x01
        #define REG_PORT_CTRL_DISABLE                     0x03

        #define REG_PORT_STP_SHIFT                        5
        #define REG_PORT_STP_MASK                         (0x7 << REG_PORT_STP_SHIFT)
        
#if defined (CONFIG_BCM963268)
    #define EPHY_PORTS       8
    #define GPHY_PORT_ID     3
    #define GPHY_PORT_PHY_ID 4
    #define NUM_RGMII_PORTS  4
    #define RGMII_PORT_ID    4
#elif defined (CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM94908)
    #define EPHY_PORTS       0
    #define NUM_RGMII_PORTS  4
    #define RGMII_PORT_ID    5
#elif defined (CONFIG_BCM963381)
    #define EPHY_PORTS       5
    #define RGMII_PORT_ID    4
    #define NUM_RGMII_PORTS  1
#elif defined (CONFIG_BCM947189)
    #define EPHY_PORTS       0
    #define NUM_RGMII_PORTS  2
    #define RGMII_PORT_ID    0
#endif

#define USB_PORT_ID     6
#define MIPS_PORT_ID    8
#define EXTSW_RGMII_PORT 5

#define BCM50210_PHYID1  0x600D
#define BCM50210_PHYID2  0x84A0
#define BCM54610_PHYID2  0xBD63
#define BCM50612_PHYID2  0x5E60
#define BCMAC201_PHYID2  0xBC30
#define BCM54616_PHYID2  0x5D10
#define BCM_PHYID_M      0xFFF0

    #define REG_MII_PORT_CONTROL                          0x08

        #define REG_MII_PORT_CONTROL_RX_UCST_EN           0x10
        #define REG_MII_PORT_CONTROL_RX_MCST_EN           0x08
        #define REG_MII_PORT_CONTROL_RX_BCST_EN           0x04
        #define REG_MII_PORT_CONTROL_TX_DISABLE           0x02
        #define REG_MII_PORT_CONTROL_RX_DISABLE           0x01

    #define REG_SWITCH_MODE                               0x0b

        #define REG_SWITCH_MODE_FRAME_MANAGE_MODE         0x01
        #define REG_SWITCH_MODE_SW_FWDG_EN                0x02
        #define REG_SWITCH_MODE_FLSH_GPON_EGRESS_Q        0x08
        #define BROADCAST_TO_ONLY_MIPS                    0x20

    #define REG_CONTROL_MII1_PORT_STATE_OVERRIDE          0x0e  // 63158: move to E4-10
        // use PORT_OVERIDE_PAGE, PORT_OVERIDE_REG(IMP_PORT_ID) for compatibility

        #define REG_CONTROL_MPSO_MII_SW_OVERRIDE          0x80
        #define REG_CONTROL_MPSO_2000M_MII                0x40 /* Valid only for SF2; Reserved for others */
        #define REG_CONTROL_MPSO_FLOW_CONTROL             0x30
        /* Below two bits : bit03:02 for speed */
        #define REG_CONTROL_MPSO_SPEED100                 0x04
        #define REG_CONTROL_MPSO_SPEED1000                0x08
        #define REG_CONTROL_MPSO_FDX                      0x02
        #define REG_CONTROL_MPSO_LINKPASS                 0x01
        #define IMP_LINK_OVERRIDE               (REG_CONTROL_MPSO_MII_SW_OVERRIDE |  \
                                                 REG_PORT_STATE_FDX | REG_PORT_STATE_LNK)
        #define IMP_LINK_OVERRIDE_1000FDX       (IMP_LINK_OVERRIDE | \
                                                 REG_CONTROL_MPSO_SPEED1000)
        #define IMP_LINK_OVERRIDE_2000FDX       (IMP_LINK_OVERRIDE_1000FDX | \
                                                 REG_CONTROL_MPSO_2000M_MII)
    #define REG_PORT_FORWARD                              0x21
        #define REG_PORT_FORWARD_MCST                     0x80
        #define REG_PORT_FORWARD_UCST                     0x40
        #define REG_PORT_FORWARD_IP_MCST                  0x01

    #define REG_PORT_ENABLE                               0x23

    #define REG_PROTECTED_PORT_MAP                        0x24 

    #define REG_WAN_PORT_MAP                              0x26
        #define EN_MAN_TO_WAN                             (1<<9)

    #define REG_PAUSE_CAPBILITY                           0x28  // 63158: move to E4-18
        // use PAUSE_CAP_PAGE, PAUSE_CAP_REG for compatibility

        #define REG_PAUSE_CAPBILITY_OVERRIDE              (1<<23)
        #define REG_PAUSE_CAPBILITY_MIPS_RX               (1<<17)
        #define REG_PAUSE_CAPBILITY_SAR_RX                (1<<16)
        #define REG_PAUSE_CAPBILITY_USB_RX                (1<<15)
        #define REG_PAUSE_CAPBILITY_GMII1_RX              (1<<14)
        #define REG_PAUSE_CAPBILITY_GMII0_RX              (1<<13)
        #define REG_PAUSE_CAPBILITY_EPHY3_RX              (1<<12)
        #define REG_PAUSE_CAPBILITY_EPHY2_RX              (1<<11)
        #define REG_PAUSE_CAPBILITY_EPHY1_RX              (1<<10)
        #define REG_PAUSE_CAPBILITY_EPHY0_RX              (1<<9)
        #define REG_PAUSE_CAPBILITY_MIPS_TX               (1<<8)
        #define REG_PAUSE_CAPBILITY_SAR_TX                (1<<7)
        #define REG_PAUSE_CAPBILITY_USB_TX                (1<<6)
        #define REG_PAUSE_CAPBILITY_GMII1_TX              (1<<5)
        #define REG_PAUSE_CAPBILITY_GMII0_TX              (1<<4)
        #define REG_PAUSE_CAPBILITY_EPHY3_TX              (1<<3)
        #define REG_PAUSE_CAPBILITY_EPHY2_TX              (1<<2)
        #define REG_PAUSE_CAPBILITY_EPHY1_TX              (1<<1)
        #define REG_PAUSE_CAPBILITY_EPHY0_TX              (1<<0)

    #define REG_UCST_LOOKUP_FAIL                          0x32 /* 5397 only */

    #define REG_MCST_LOOKUP_FAIL                          0x34 /* 5397 only */

    #define REG_IPMC_LOOKUP_FAIL                          0x36 /* 5397 only */

    #define REG_DISABLE_LEARNING                          0x3c

    #define REG_LOW_POWER_EXP1                                0x40

        #define REG_LOW_POWER_EXP1_SLEEP_MACCLK_PORT_SHIFT    16
        #define REG_LOW_POWER_EXP1_SLEEP_MACCLK_PORT_MASK     (0x1FF << REG_LOW_POWER_EXP1_SLEEP_MACCLK_PORT_SHIFT)

    #define REG_PORT_STATE                                0x58  // 63158: move to E4-00
        // use PORT_OVERIDE_PAGE, PORT_OVERIDE_REG(n) for compatibility

        #define REG_PORT_STATE_RESERVE_0                  0xffffff00  /* Read unknown, write must be 0 */
        #define REG_PORT_GMII_SPEED_UP_2G                 0x80 /* Valid only for SF2 (4908 and above); Reserved for others */
        #define REG_PORT_STATE_OVERRIDE                   0x40
        #define REG_PORT_STATE_TX_FLOWCTL                 0x20
        #define REG_PORT_STATE_RX_FLOWCTL                 0x10
        #define REG_PORT_STATE_100                        0x04
        #define REG_PORT_STATE_1000                       0x08
        #define REG_PORT_STATE_FDX                        0x02
        #define REG_PORT_STATE_LNK                        0x01
        #define LINK_OVERRIDE_1000FDX  (REG_PORT_STATE_OVERRIDE |  \
            REG_PORT_STATE_1000 | REG_PORT_STATE_FDX | REG_PORT_STATE_LNK)
    #define REG_RGMII_CTRL_P4                             0x64
    #define REG_RGMII_CTRL_P5                             0x65
    #define REG_RGMII_CTRL_P7                             0x67
        #define REG_RGMII_CTRL_ENABLE_GMII                0x80
        #define REG_RGMII_CTRL_DLL_IQQD                   0x04
        #define REG_RGMII_CTRL_DLL_RXC_BYPASS             0x02
        #define REG_RGMII_CTRL_TIMING_SEL                 0x01
        #define REG_RGMII_CTRL_ENABLE_RGMII_OVERRIDE      0x40 
        #define REG_RGMII_CTRL_MODE                       0x30
            #define REG_RGMII_CTRL_MODE_RGMII             0x00
            #define REG_RGMII_CTRL_MODE_RvMII             0x20
            #define REG_RGMII_CTRL_MODE_MII               0x10
            #define REG_RGMII_CTRL_MODE_GMII              0x30

/*   External switch RGMII definitions */

    #define EXTSW_REG_RGMII_CTRL_IMP                      0x60
        #define EXTSW_RGMII_TXCLK_DELAY                   0x01
        #define EXTSW_RGMII_RXCLK_DELAY                   0x02

    #define REG_RGMII_TIMING_P4                           0x6c
    #define REG_RGMII_TIMING_P5                           0x6d
        #define RGMII_EN_DELAY_WRITE                      0x80
        #define RGMII_DELAY_VAL_M                         0x7
        #define RGMII_DELAY_VAL_S                         4
            #define RGMII_DELAY_VAL2                      0x7
        #define RGMII_DELAY_SEL_M                         0xF
        #define RGMII_DELAY_SEL_S                         0
            #define RGMII_DELAY_CLK_GTXR                  0x9
            #define RGMII_DELAY_CLK_RXR                   0xb

    #define REG_SW_RESET                                  0x79
        #define REG_SW_RST                                0x80
        #define REG_EN_RESET_REG                          0x02
        #define REG_EN_RESET_SW_CORE                      0x01

    #define REG_FAST_AGING_CTRL                           0x88
        #define FAST_AGE_START_DONE                       0x80
        #define FAST_AGE_DYNAMIC                          0x02
        #define FAST_AGE_STATIC                           0x01
        #define FAST_AGE_PORT                             0x04
        #define FAST_AGE_VLAN                             0x08
        #define FAST_AGE_SPT                              0x10
    #define REG_FAST_AGING_PORT                           0x89

    #define REG_SWPKT_CTRL_USB                            0xa0
    #define REG_SWPKT_CTRL_GPON                           0xa4

    #define REG_IUDMA_CTRL                                0xa8

        #define REG_IUDMA_CTRL_USE_QUEUE_PRIO             0x0400
        #define REG_IUDMA_CTRL_USE_DESC_PRIO              0x0200
        #define REG_IUDMA_CTRL_TX_DMA_EN                  0x0100
        #define REG_IUDMA_CTRL_RX_DMA_EN                  0x0080
        #define REG_IUDMA_CTRL_PRIO_SEL                   0x0040
        #define REG_IUDMA_CTRL_TX_MII_TAG_DISABLE         0x1000		

    #define REG_IUDMA_QUEUE_CTRL                          0xac
        #define REG_IUDMA_Q_CTRL_RXQ_SEL_S                16
        #define REG_IUDMA_Q_CTRL_RXQ_SEL_M                0xFFFF
        #define REG_IUDMA_Q_CTRL_PRIO_TO_CH_M             0x3
        #define REG_IUDMA_Q_CTRL_TXQ_SEL_S                0
        #define REG_IUDMA_Q_CTRL_TXQ_SEL_M                0xFFF
        #define REG_IUDMA_Q_CTRL_CH_TO_PRIO_M             0x7

    #define REG_MDIO_CTRL_ADDR                            0xb0
        #define REG_MDIO_CTRL_WRITE                       (1 << 31)
        #define REG_MDIO_CTRL_READ                        (1 << 30)
        #define REG_MDIO_CTRL_EXT                         (1 << 16)
        #define REG_MDIO_CTRL_EXT_BIT(id)                 (IsExtPhyId(id)? REG_MDIO_CTRL_EXT: 0)
        #define REG_MDIO_CTRL_ID_SHIFT                    25
        #define REG_MDIO_CTRL_ID_MASK                     (0x1f << REG_MDIO_CTRL_ID_SHIFT)
        #define REG_MDIO_CTRL_ADDR_SHIFT                  20
        #define REG_MDIO_CTRL_ADDR_MASK                   (0x1f << REG_MDIO_CTRL_ADDR_SHIFT)

    #define REG_MDIO_DATA_ADDR                            0xb4

    #define REG_EEE_CTRL                                  0xc0
        #define REG_EEE_CTRL_ENABLE                       0x01

    #define REG_EEE_TW_SYS_TX_100                         0xd0
    #define REG_EEE_TW_SYS_TX_1000                        0xd2

    #define REG_LOW_POWER_CTRL                                0xDE

        #define REG_LOW_POWER_CTR_SLEEP_P8                    0x8000
        #define REG_LOW_POWER_CTR_SLEEP_P5                    0x1000
        #define REG_LOW_POWER_CTR_SLEEP_P4                    0x0800
        #define REG_LOW_POWER_CTR_SLEEP_SYS                   0x0040
        #define REG_LOW_POWER_CTR_TIMER_DISABLE               0x0020
        #define REG_LOW_POWER_CTR_EN_LOW_POWER                0x0010
        #define REG_LOW_POWER_CTR_LOW_POWER_DIVIDER_12P5MHZ   0x0000
        #define REG_LOW_POWER_CTR_LOW_POWER_DIVIDER_6P25MHZ   0x0001
        #define REG_LOW_POWER_CTR_LOW_POWER_DIVIDER_4P17MHZ   0x0002
        #define REG_LOW_POWER_CTR_LOW_POWER_DIVIDER_3P125MHZ  0x0003

/****************************************************************************
    External switch pseudo PHY: Page (0x02)
****************************************************************************/

#define PAGE_MANAGEMENT                                   0x02

    #define REG_GLOBAL_CONFIG                             0x00

        #define ENABLE_MII_PORT                           0x80
        #define IGMP_REDIRECTION_EN                       0x01
        #define RECEIVE_IGMP                              0x08
        #define RECEIVE_BPDU                              0x02
        #define GLOBAL_CFG_RESET_MIB                      0x01

    #define REG_BRCM_HDR_CTRL                             0x03

        #define BRCM_HDR_EN_GMII_PORT_7                   0x04
        #define BRCM_HDR_EN_GMII_PORT_5                   0x02
        #define BRCM_HDR_EN_IMP_PORT                      0x01  /* Port8 */

    #define REG_AGING_TIME_CTRL                           0x08

        #define REG_AGE_CHANGE                            0x100000
        #define REG_AGE_TIME_MASK                         0x0FFFFF

    #define REG_IPG_SHRNK_CTRL                            0x0c
        #define IPG_NO_SHRNK                              0
        #define IPG_1BYTE_SHRNK                           1
        #define IPG_4BYTE_SHRNK                           2
        #define IPG_5BYTE_SHRNK                           3
        #define IPG_SHRNK_VAL(p,v)                        ((v)<<((p)*2)) 
        #define IPG_SHRNK_MASK(p)                         (3UL<<((p)*2))

    #define REG_MIRROR_CAPTURE_CTRL                       0x10

        #define REG_MIRROR_ENABLE                         0x8000
        #define REG_BLK_NOT_MIRROR                        0x4000
        #define REG_CAPTURE_PORT_M                        0xF

    #define REG_MIRROR_INGRESS_CTRL                       0x12

        #define REG_INGRESS_MIRROR_M                      0x1FF

    #define REG_MIRROR_EGRESS_CTRL                        0x1c

        #define REG_EGRESS_MIRROR_M                       0x1FF

    #define REG_DEV_ID                                    0x30

    #define REG_RST_MIB_CNT_EN                            0x54

        #define REG_RST_MIB_CNT_EN_PORT_M                 0x1FF

/****************************************************************************
    ARL CTRL Regsiters: Page (0x04)
****************************************************************************/

#define PAGE_ARLCTRL                                      0x04

    #define REG_ARLCFG                                    0x00
        #define MULTIPORT_ADDR_EN_M                       0x1
        #define MULTIPORT_ADDR_EN_S                       4

    /* external 53115 and 53125 switch mport control register */
    #define REG_MULTIPORT_CTRL                            0xe
        #define MULTIPORT_CTRL_DA_HIT_EN                  14
        #define MULTIPORT_CTRL_COUNT                      6
        #define MULTIPORT_CTRL_ETYPE_CMP                  0x1
        #define MULTIPORT_CTRL_ADDR_CMP                   0x2
        #define MULTIPORT_CTRL_ADDR_ETYPE_CMP             0x3
        #define MULTIPORT_CTRL_EN_M                       0x3

    #define REG_MULTIPORT_ADDR1_LO                        0x10
    #define REG_MULTIPORT_ADDR1_HI                        0x14
    #define REG_MULTIPORT_VECTOR1                         0x18
    #define REG_MULTIPORT_ADDR2_LO                        0x20
    #define REG_MULTIPORT_ADDR2_HI                        0x24
    #define REG_MULTIPORT_VECTOR2                         0x28

/****************************************************************************
    ARL/VLAN Table Access: Page (0x05)
****************************************************************************/

#define PAGE_AVTBL_ACCESS                                 0x05

    #define REG_ARL_TBL_CTRL                              0x00
        #define ARL_TBL_CTRL_START_DONE                   0x80
        #define ARL_TBL_CTRL_READ                         0x01
    #define REG_ARL_MAC_INDX_LO                           0x02
    #define REG_ARL_MAC_INDX_HI                           0x04
    #define REG_ARL_VLAN_INDX                             0x08
    #define REG_ARL_MAC_LO_ENTRY                          0x10
    #define REG_ARL_VID_MAC_HI_ENTRY                      0x14
    #define REG_ARL_DATA_ENTRY                            0x18
        #define ARL_DATA_ENTRY_VALID                      0x8000
        #define ARL_DATA_ENTRY_STATIC                     0x4000
    #define REG_ARL_BINS_PER_HASH                         4
    #define REG_ARL_SRCH_CTRL                             0x30
        #define ARL_SRCH_CTRL_START_DONE                  0x80 
        #define ARL_SRCH_CTRL_SR_VALID                    0x01
    #define REG_ARL_SRCH_ADDR                             0x32
    #define REG_ARL_SRCH_MAC_LO_ENTRY                     0x34
    #define REG_ARL_SRCH_VID_MAC_HI_ENTRY                 0x38
    #define REG_ARL_SRCH_DATA_ENTRY                       0x3C
    #define REG_VLAN_TBL_CTRL                             0x60
    #define REG_VLAN_TBL_INDX                             0x62
    #define REG_VLAN_TBL_ENTRY                            0x64

/* External switch 53115 and 53125 ARL search registers */
    #define REG_ARL_SRCH_CTRL_531xx                       0x50
    #define REG_ARL_SRCH_MAC_LO_ENTRY0_531xx              0x60
    #define REG_ARL_SRCH_VID_MAC_HI_ENTRY0_531xx          0x64
    #define REG_ARL_SRCH_DATA_ENTRY0_531xx                0x68
        #define ARL_DATA_ENTRY_VALID_531xx                0x10000
        #define ARL_DATA_ENTRY_STATIC_531xx               0x8000
    #define REG_ARL_SRCH_MAC_LO_ENTRY1_531xx               0x70
    #define REG_ARL_SRCH_VID_MAC_HI_ENTRY1_531xx           0x74
    #define REG_ARL_SRCH_DATA_ENTRY1_531xx                 0x78
    #define REG_ARL_SRCH_HASH_ENTS                        2
    #define REG_VLAN_TBL_CTRL_531xx                       0x80
    #define REG_VLAN_TBL_INDX_531xx                       0x81
    #define REG_VLAN_TBL_ENTRY_531xx                      0x83

    //#define REG_ARL_SRCH_DATA_RESULT_DONE_531xx           0x78
    
/****************************************************************************
   Flow Control: Page (0x0A)
****************************************************************************/

#define PAGE_FLOW_CTRL                                    0x0A

    #define REG_FC_DIAG_PORT_SEL                          0x00
    #define REG_FC_CTRL                                   0x0E
    #define REG_FC_PRIQ_HYST                              0x10
    #define REG_FC_PRIQ_PAUSE                             0x18
    #define REG_FC_PRIQ_DROP                              0x20
    #define REG_FC_PRIQ_TOTAL_HYST                        0x28
    #define REG_FC_PRIQ_TOTAL_PAUSE                       0x30
    #define REG_FC_PRIQ_TOTAL_DROP                        0x38
    #define REG_FC_Q_MON_CNT                              0x60
    #define REG_PEAK_RXBUF_COUNT                          0x78


/****************************************************************************
   External Switch Internal Phy: Page (0x10-0x14)
****************************************************************************/

#define PAGE_INTERNAL_PHY_MII                             0x10

/****************************************************************************
   MIB Counters: Page (0x20 to 0x28)
****************************************************************************/

#define PAGE_MIB_P0                                       0x20

    /* Transmit Stats */
    #define REG_MIB_P0_TXOCTETS                           0x00
    #define REG_MIB_P0_TXDROPS                            0x08
	#define REG_MIB_P0_TXQ0PKT							  0x0C  
    #define REG_MIB_P0_TXBPKTS                            0x10
    #define REG_MIB_P0_TXMPKTS                            0x14
    #define REG_MIB_P0_TXUPKTS                            0x18  
    #define REG_MIB_P0_TXCOL                              0x1C 
    #define REG_MIB_P0_TXSINGLECOL                        0x20 
    #define REG_MIB_P0_TXMULTICOL                         0x24
    #define REG_MIB_P0_TXDEFERREDTX                       0x28
    #define REG_MIB_P0_TXLATECOL                          0x2C
    #define REG_MIB_P0_TXEXCESSCOL                        0x30
    #define REG_MIB_P0_TXFRAMEINDISC                      0x34
    #define REG_MIB_P0_TXPAUSEPKTS                        0x38
    /* Receive stats */
    #define REG_MIB_P0_RXOCTETS                           0x44  /* Start using REG_MIB_P0_EXTSWITCH_OFFSET for External switch */
    #define REG_MIB_P0_RXUNDERSIZEPKTS                    0x4C
    #define REG_MIB_P0_RXPAUSEPKTS                        0x50  
    #define REG_MIB_P0_RX64OCTPKTS                        0x54  
    #define REG_MIB_P0_RX127OCTPKTS                       0x58  
    #define REG_MIB_P0_RX255OCTPKTS                       0x5C  
    #define REG_MIB_P0_RX511OCTPKTS                       0x60  
    #define REG_MIB_P0_RX1023OCTPKTS                      0x64 
    #define REG_MIB_P0_RXMAXOCTPKTS                       0x68 
    #define REG_MIB_P0_RXOVERSIZE                         0x6C 
    #define REG_MIB_P0_RXJABBERS                          0x70
    #define REG_MIB_P0_RXALIGNERRORS                      0x74
    #define REG_MIB_P0_RXFCSERRORS                        0x78
    #define REG_MIB_P0_RXDROPS                            0x84
    #define REG_MIB_P0_RXUPKTS                            0x88
    #define REG_MIB_P0_RXMPKTS                            0x8c
    #define REG_MIB_P0_RXBPKTS                            0x90
    #define REG_MIB_P0_RXSACHANGES                        0x94
    #define REG_MIB_P0_RXFRAGMENTS                        0x98
    #define REG_MIB_P0_RXEXCESSZISEDISC                   0x9C   /* Internal Switch only */
    #define REG_MIB_P0_RXSYMBOLERRORS                     0xA0
    #define REG_MIB_P0_RXJUMBOPKT_EXT                     0xA8   /* External Switch only */
    #define REG_MIB_P0_RXOUTOFRANGE_EXT                   0xB4   /* External Switch only */
    #define REG_MIB_P0_RXDISCARD_EXT                      0xC0   /* External Switch only */

    /* Define an offset for use for register reads of external switches */
    /* NOTE: The 5398 is the same as the itnernal Roboswitch, but on
       legacy designs can actually be an external switch. */
    #if (CONFIG_BCM_EXT_SWITCH_TYPE == 5398)
        #define REG_MIB_P0_EXTSWITCH_OFFSET               0x00
    #else
        #define REG_MIB_P0_EXTSWITCH_OFFSET               0x0C
    #endif
        
// External switch MIB has more counters

    #define REG_MIB_P0_EXT_TXOCTETS                           0x00
    #define REG_MIB_P0_EXT_TXDROPS                            0x08
    #define REG_MIB_P0_EXT_TXQ0PKT                            0x0C  
    #define REG_MIB_P0_EXT_TXBPKTS                            0x10
    #define REG_MIB_P0_EXT_TXMPKTS                            0x14
    #define REG_MIB_P0_EXT_TXUPKTS                            0x18  
    #define REG_MIB_P0_EXT_TXCOL                              0x1C 
    #define REG_MIB_P0_EXT_TXSINGLECOL                        0x20 
    #define REG_MIB_P0_EXT_TXMULTICOL                         0x24
    #define REG_MIB_P0_EXT_TXDEFERREDTX                       0x28
    #define REG_MIB_P0_EXT_TXLATECOL                          0x2C
    #define REG_MIB_P0_EXT_TXEXCESSCOL                        0x30 /* Defined for 53125 but not for 53101 */
    #define REG_MIB_P0_EXT_TXFRAMEINDISC                      0x34
    #define REG_MIB_P0_EXT_TXPAUSEPKTS                        0x38
    #define REG_MIB_P0_EXT_TXQ1PKT                            0x3c
    #define REG_MIB_P0_EXT_TXQ2PKT                            0x40
    #define REG_MIB_P0_EXT_TXQ3PKT                            0x44
    #define REG_MIB_P0_EXT_TXQ4PKT                            0x48
    #define REG_MIB_P0_EXT_TXQ5PKT                            0x4c
    #define REG_MIB_P0_EXT_RXOCTETS                           0x50  
    #define REG_MIB_P0_EXT_RXUNDERSIZEPKTS                    0x58
    #define REG_MIB_P0_EXT_RXPAUSEPKTS                        0x5c  
    #define REG_MIB_P0_EXT_RX64OCTPKTS                        0x60  
    #define REG_MIB_P0_EXT_RX127OCTPKTS                       0x64  
    #define REG_MIB_P0_EXT_RX255OCTPKTS                       0x68  
    #define REG_MIB_P0_EXT_RX511OCTPKTS                       0x6c  
    #define REG_MIB_P0_EXT_RX1023OCTPKTS                      0x70 
    #define REG_MIB_P0_EXT_RXMAXOCTPKTS                       0x74 
    #define REG_MIB_P0_EXT_RXOVERSIZE                         0x78 
    #define REG_MIB_P0_EXT_RXJABBERS                          0x7c
    #define REG_MIB_P0_EXT_RXALIGNERRORS                      0x80
    #define REG_MIB_P0_EXT_RXFCSERRORS                        0x84
    #define REG_MIB_P0_EXT_RXGOODOCT                          0x88
    #define REG_MIB_P0_EXT_RXDROPS                            0x90
    #define REG_MIB_P0_EXT_RXUPKTS                            0x94
    #define REG_MIB_P0_EXT_RXMPKTS                            0x98
    #define REG_MIB_P0_EXT_RXBPKTS                            0x9c
    #define REG_MIB_P0_EXT_RXSACHANGES                        0xa0
    #define REG_MIB_P0_EXT_RXFRAGMENTS                        0xa4
    #define REG_MIB_P0_EXT_RXJUMBOPKT                         0xa8 
    #define REG_MIB_P0_EXT_RXSYMBOLERRORS                     0xAc
    #define REG_MIB_P0_EXT_RXINRANGEERR                       0xB0
    #define REG_MIB_P0_EXT_RXOUTRANGEERR                      0xB4
    #define REG_MIB_P0_EXT_EEELPIEVEVT                        0xB8  /* Defined for 53125 but not for 53101 */
    #define REG_MIB_P0_EXT_EEELPIDURATION                     0xBc  /* Defined for 53125 but not for 53101 */
    #define REG_MIB_P0_EXT_RXDISCARD                          0xC0
    
/****************************************************************************
   QOS : Page (0x30)
****************************************************************************/
#if defined (CONFIG_BCM963158)
    #define QOS_REG_SHIFT       4   // reg 0x28 to 0x64 need to shift 4 bytes
#else
    #define QOS_REG_SHIFT       0
#endif

#define PAGE_QOS                                          0x30

    #define REG_QOS_GLOBAL_CTRL                           0x00
        #define PORT_QOS_EN_M                             0x1
        #define PORT_QOS_EN_S                             6
        #define QOS_LAYER_SEL_M                           0x3
        #define QOS_LAYER_SEL_S                           2
        #define MIB_QOS_MONITOR_SET_M                     0x3
        #define MIB_QOS_MONITOR_SET_S                     0
    #define REG_QOS_THRESHOLD_CTRL                        0x02
        #define AUTOSET_QOS_EN                            0x2000
 /* The per port 802.1p QOS Enable Register */
    #define REG_QOS_8021P_EN                              0x04
 /* The per port DSCP QOS Enable Register */
    #define REG_QOS_DSCP_EN                               0x06

    #define REG_QOS_8021P_PRIO_MAP                        0x10
    #define REG_QOS_8021P_PRIO_MAP_IP                     (0x28+QOS_REG_SHIFT)
    #define REG_QOS_PORT_PRIO_MAP_P0                      (0x50+QOS_REG_SHIFT)
    #define REG_QOS_PORT_PRIO_MAP_EXT                     (0x62+QOS_REG_SHIFT)
    #define COSQ_PORT_MAPPING_REG_LEN_4Q                  2
    #define COSQ_PORT_MAPPING_REG_LEN_8Q                  4
    #define REG_QOS_PRIO_TO_QID_SEL_BITS                  2
    #define REG_QOS_PRIO_TO_QID_SEL_M                     0x3

    #define REG_QOS_DSCP_PRIO_MAP0LO                      (0x30+QOS_REG_SHIFT)
    #define REG_QOS_DSCP_PRIO_MAP0HI                      (0x34+QOS_REG_SHIFT)
    #define REG_QOS_DSCP_PRIO_MAP1LO                      (0x36+QOS_REG_SHIFT)
    #define REG_QOS_DSCP_PRIO_MAP1HI                      (0x38+QOS_REG_SHIFT)
    #define REG_QOS_DSCP_PRIO_MAP2LO                      (0x3c+QOS_REG_SHIFT)
    #define REG_QOS_DSCP_PRIO_MAP2HI                      (0x40+QOS_REG_SHIFT)
    #define REG_QOS_DSCP_PRIO_MAP3LO                      (0x42+QOS_REG_SHIFT)
    #define REG_QOS_DSCP_PRIO_MAP3HI                      (0x44+QOS_REG_SHIFT)

    #define QOS_DSCP_M                                    0x3F
    #define QOS_TC_S                                      0x3
    #define QOS_TC_M                                      0x7U
    #define QOS_DSCP_MAP_M                                0x3
    #define QOS_DSCP_MAP_S                                0x4
    #define QOS_DSCP_MAP_LSBITS_M                         0xF
    #define QOS_DSCP_MAP_REG_SZ                           0x6
    #define QOS_PCP_MAP_REG_SZ                            0x4

    #define REG_QOS_TXQ_CTRL                              0x80
        #define TXQ_CTRL_TXQ_MODE_M                       0x3 
        #define TXQ_CTRL_TXQ_MODE_S                       2 
        #define TXQ_CTRL_HQ_PREEMPT_M                     0x1
        #define TXQ_CTRL_HQ_PREEMPT_S                     4 
        #define TXQ_CTRL_TXQ_MODE_EXT_M                   0x3 
        #define TXQ_CTRL_TXQ_MODE_EXT_S                   0 
    #define REG_QOS_TXQ_WEIGHT_Q0                         0x81
    #define REG_QOS_VID_REMAP                             0xB0
    #define REG_QOS_VID_REMAP_CTRL                        0xD0

/****************************************************************************
    Port-Based VLAN Control: Page (0x31)
****************************************************************************/

#define PAGE_PORT_BASED_VLAN                              0x31

    #define REG_VLAN_CTRL_P0                              0x00
    #define REG_VLAN_CTRL_P1                              0x02
    #define REG_VLAN_CTRL_P2                              0x04
    #define REG_VLAN_CTRL_P3                              0x06
    #define REG_VLAN_CTRL_P4                              0x08
    #define REG_VLAN_CTRL_P5                              0x0a
    #define REG_VLAN_CTRL_P6                              0x0c
    #define REG_VLAN_CTRL_P7                              0x0e
    #define REG_VLAN_CTRL_P8                              0x10

/****************************************************************************
    MAC-BASED Port Trunking (LAG - Link Aggregation) : Page (0x32)
****************************************************************************/

#define PAGE_MAC_BASED_TRUNK                              0x32

    #define REG_MAC_TRUNK_CTL                             0x00
        #define TRUNK_EN_LOCAL_M                          0x1
        #define TRUNK_EN_LOCAL_S                          3
        #define TRUNK_HASH_SEL_M                          0x3
        #define TRUNK_HASH_SEL_S                          0
            #define TRUNK_HASH_DA_SA_VID                  0x00
            #define TRUNK_HASH_DA_VID                     0x01
            #define TRUNK_HASH_SA_VID                     0x02
    #define REG_TRUNK_GRP_CTL                             0x10 /* Two groups - 2 Byte each */
        #define TRUNK_EN_GRP_M                            0x1FF
        #define TRUNK_EN_GRP_S                            0

/****************************************************************************
   802.1Q VLAN Control: Page (0x34)
****************************************************************************/

#define PAGE_8021Q_VLAN                                   0x34

    #define REG_VLAN_GLOBAL_8021Q                         0x00
        #define VLAN_EN_8021Q_M                           0x1
        #define VLAN_EN_8021Q_S                           7
        #define VLAN_IVL_SVL_M                            0x3
        #define VLAN_IVL_SVL_S                            5
        #define PRIO_TAG_FRAME_CTRL_M                     0x3
        #define PRIO_TAG_FRAME_CTRL_S                     0
    #define REG_VLAN_GLOBAL_CTRL1                         0x01
    #define REG_VLAN_GLOBAL_CTRL2                         0x02
    #define REG_VLAN_GLOBAL_CTRL3                         0x04
    #define REG_VLAN_GLOBAL_CTRL4                         0x06
    #define REG_VLAN_GLOBAL_CTRL5                         0x07
        #define TAG_STATUS_PRESERVE_M                     0x3
        #define TAG_STATUS_PRESERVE_S                     5
        #define TAG_STATUS_PRESERVE_01                    1
        #define TAG_STATUS_PRESERVE_10                    2
        #define VID_FFF_ENABLE_M                          0x1
        #define VID_FFF_ENABLE_S                          2
    #define REG_VLAN_MULTI_PORT_ADDR_CTL                  0x0A
        #define EN_MPORT_V_U_FWD_MAP_S                    (2)
        #define EN_MPORT_V_FWD_MAP                        (1<<0)
        #define EN_MPORT_U_FWD_MAP                        (1<<1)
    #define REG_VLAN_8021P_REMAP                          0x0c
    #define REG_VLAN_DEFAULT_TAG                          0x10
        #define DEFAULT_TAG_PRIO_M                        0x7
        #define DEFAULT_TAG_PRIO_S                        13
        #define DEFAULT_TAG_CFI_M                         0x1
        #define DEFAULT_TAG_CFI_S                         12
        #define DEFAULT_TAG_VID_M                         0xFFF
        #define DEFAULT_TAG_VID_S                         0
    #define REG_VLAN_DOUBLE_TAG_TPID                      0x30
    #define REG_VLAN_ISP_PORT_SEL                         0x32

/****************************************************************************
   DOS Prevent Register: Page (0x36)
****************************************************************************/

#define PAGE_DOS_PREVENT_531xx                            0x36

    #define REG_DOS_CTRL			                      0x00
		#define DOS_CTRL_M								  0x0001 /* Defining common mask for all */
		#define DOS_CTRL_RSVD_S							  0
		#define DOS_CTRL_RSVD							  (DOS_CTRL_M << DOS_CTRL_RSVD_S)
		#define IP_LAN_DROP_EN_S						  1
		#define IP_LAN_DROP_EN							  (DOS_CTRL_M << IP_LAN_DROP_EN_S)
		#define TCP_BLAT_DROP_EN_S						  2
		#define TCP_BLAT_DROP_EN						  (DOS_CTRL_M << TCP_BLAT_DROP_EN_S)
		#define UDP_BLAT_DROP_EN_S						  3
		#define UDP_BLAT_DROP_EN						  (DOS_CTRL_M << UDP_BLAT_DROP_EN_S)
		#define TCP_NULL_SCAN_DROP_EN_S					  4
		#define TCP_NULL_SCAN_DROP_EN					  (DOS_CTRL_M << TCP_NULL_SCAN_DROP_EN_S)
		#define TCP_XMAS_SCAN_DROP_EN_S					  5
		#define TCP_XMAS_SCAN_DROP_EN					  (DOS_CTRL_M << TCP_XMAS_SCAN_DROP_EN_S)
		#define TCP_SYNFIN_SCAN_DROP_EN_S				  6
		#define TCP_SYNFIN_SCAN_DROP_EN					  (DOS_CTRL_M << TCP_SYNFIN_SCAN_DROP_EN_S)
		#define TCP_SYNERR_SCAN_DROP_EN_S				  7
		#define TCP_SYNERR_SCAN_DROP_EN					  (DOS_CTRL_M << TCP_SYNERR_SCAN_DROP_EN_S)
		#define TCP_SHORTHDR_SCAN_DROP_EN_S				  8
		#define TCP_SHORTHDR_SCAN_DROP_EN				  (DOS_CTRL_M << TCP_SHORTHDR_SCAN_DROP_EN_S)
		#define TCP_FRAGERR_SCAN_DROP_EN_S				  9
		#define TCP_FRAGERR_SCAN_DROP_EN				  (DOS_CTRL_M << TCP_FRAGERR_SCAN_DROP_EN_S)
		#define ICMPv4_FRAG_DROP_EN_S				 	  10
		#define ICMPv4_FRAG_DROP_EN						  (DOS_CTRL_M << ICMPv4_FRAG_DROP_EN_S)
		#define ICMPv6_FRAG_DROP_EN_S				  	  11
		#define ICMPv6_FRAG_DROP_EN				  		  (DOS_CTRL_M << ICMPv6_FRAG_DROP_EN_S)
		#define ICMPv4_LONGPING_DROP_EN_S				  12
		#define ICMPv4_LONGPING_DROP_EN					  (DOS_CTRL_M << ICMPv4_LONGPING_DROP_EN_S)
		#define ICMPv6_LONGPING_DROP_EN_S				   13
		#define ICMPv6_LONGPING_DROP_EN				  	  (DOS_CTRL_M << ICMPv6_LONGPING_DROP_EN_S)

	#define REG_DOS_DISABLE_LRN							  0x10
		#define DOS_DISABLE_LRN_M						  0x01
		#define DOS_DISABLE_LRN_S						  0
		#define DOS_DISABLE_LRN							  (DOS_DISABLE_LRN_M << DOS_DISABLE_LRN_S)

/****************************************************************************
   Jumbo Frame Control: Page (0x40)
****************************************************************************/

#define PAGE_JUMBO                                        0x40

    #define REG_JUMBO_PORT_MASK_INT_SW                    0x04
    #define REG_JUMBO_FRAME_SIZE_INT_SW                   0x08
    #define REG_JUMBO_PORT_MASK                           0x1
    #define REG_JUMBO_FRAME_SIZE                          0x5

    #define MAX_HW_JUMBO_FRAME_SIZE                       0x2600 /* 9278 */ 

    // Defines for JUMBO register control
    #define ETHSWCTL_JUMBO_PORT_ALL                       9
    #define ETHSWCTL_JUMBO_PORT_MIPS                      8
    #define ETHSWCTL_JUMBO_PORT_GPON                      7
    #define ETHSWCTL_JUMBO_PORT_USB                       6
    #define ETHSWCTL_JUMBO_PORT_MOCA                      5
    #define ETHSWCTL_JUMBO_PORT_GPON_SERDES               4
    #define ETHSWCTL_JUMBO_PORT_GMII_2                    3
    #define ETHSWCTL_JUMBO_PORT_GMII_1                    2
    #define ETHSWCTL_JUMBO_PORT_GPHY_1                    1
    #define ETHSWCTL_JUMBO_PORT_GPHY_0                    0

    #define ETHSWCTL_JUMBO_PORT_MIPS_MASK                 0x0100
    #define ETHSWCTL_JUMBO_PORT_GPON_MASK                 0x0080
    #define ETHSWCTL_JUMBO_PORT_USB_MASK                  0x0040
    #define ETHSWCTL_JUMBO_PORT_MOCA_MASK                 0x0020
    #define ETHSWCTL_JUMBO_PORT_GPON_SERDES_MASK          0x0010
    #define ETHSWCTL_JUMBO_PORT_GMII_2_MASK               0x0008
    #define ETHSWCTL_JUMBO_PORT_GMII_1_MASK               0x0004
    #define ETHSWCTL_JUMBO_PORT_GPHY_1_MASK               0x0002
    #define ETHSWCTL_JUMBO_PORT_GPHY_0_MASK               0x0001
    #define ETHSWCTL_JUMBO_PORT_MASK_VAL                  0x01FF

/****************************************************************************
   Broadcast Storm Suprression Registers
****************************************************************************/

#define PAGE_BSS                                          0x41

    #define REG_BSS_IRC_CONFIG                            0x00
        #define IRC_CFG_XLENEN                            16
        #define IRC_CFG_PKT_MSK0_EXT_S                    17

    #define REG_BSS_RX_RATE_CTRL_P0                       0x10
        #define IRC_BKT0_RATE_CNT_M                       0xFF
        #define IRC_BKT0_RATE_CNT_S                       0
        #define IRC_BKT0_SIZE_M                           0x7
        #define IRC_BKT0_SIZE_S                           8
        #define IRC_BKT0_EN_M                             0x01
        #define IRC_BKT0_EN_S                             22

    #define REG_BSS_PKT_DROP_CNT_P0                       0x50

    #define REG_BSS_TX_RATE_CTRL_P0                       0x80
        #define ERC_RFSH_CNT_M                            0xFF
        #define ERC_RFSH_CNT_S                            0
        #define ERC_BKT_SIZE_M                            0x7
        #define ERC_BKT_SIZE_S                            8
        #define ERC_ERC_EN_M                              0x01
        #define ERC_ERC_EN_S                              11


/****************************************************************************
   EEE: Page (0x92)
****************************************************************************/

#define PAGE_EEE                                          0x92
    #define REG_EEE_EN_CTRL                               0x00
    #define REG_EEE_SLEEP_TIMER_G                         0x10
    #define REG_EEE_SLEEP_TIMER_FE                        0x34

/****************************************************************************
   FFE Registers
****************************************************************************/

#define PAGE_FFE                                          0xB4
    #define REG_FFE_RXPORT_CTRL                           0x1C
	    #define FFE_PAD_ENABLE_M                          0x1
	    #define FFE_PAD_ENABLE_S                          8
	    #define FFE_PAD_SIZE_M                            0xFF
	    #define FFE_PAD_SIZE_S                            0

#if defined (CONFIG_BCM963158)
/****************************************************************************
   63158 expanded page_control Registers
****************************************************************************/

#define PAGE_CONTROL_EXP                                  0xE4

    #define REG_PORT0_STATE_EXP                           0x00  // see REG_PORT_STATE bit definitions
    #define REG_PORT1_STATE_EXP                           0x02
    #define REG_PORT2_STATE_EXP                           0x04
    #define REG_PORT3_STATE_EXP                           0x06
    #define REG_PORT4_STATE_EXP                           0x08
    #define REG_PORT5_STATE_EXP                           0x0a
    #define REG_PORT6_STATE_EXP                           0x0c
    #define REG_PORT7_STATE_EXP                           0x0e
    #define REG_PORT8_STATE_EXP                           0x10

        #define REG_PORT0_STATE_EXP_SHIFT               2

    #define REG_PAUSE_CAPBILITY_EXP                       0x18  // see REG_PAUSE_CAPBILITY bit definitions

#define PORT_OVERIDE_PAGE       PAGE_CONTROL_EXP
#define PORT_OVERIDE_REG(p)     (REG_PORT0_STATE_EXP+(p)*REG_PORT0_STATE_EXP_SHIFT)

#define PAUSE_CAP_PAGE          PAGE_CONTROL_EXP
#define PAUSE_CAP_REG           REG_PAUSE_CAPBILITY_EXP

#else   // !BCM963158

#define PORT_OVERIDE_PAGE       PAGE_CONTROL
#define PORT_OVERIDE_REG(p)     (((p)==IMP_PORT_ID)?REG_CONTROL_MII1_PORT_STATE_OVERRIDE:REG_PORT_STATE+(p))

#define PAUSE_CAP_PAGE          PAGE_CONTROL
#define PAUSE_CAP_REG           REG_PAUSE_CAPBILITY

#endif // !BCM963158

/****************************************************************************
   Shared Defines for bcmsw.c and ethsw.c files
****************************************************************************/
#define TOTAL_SWITCH_PORTS   9
#define PBMAP_ALL            0x1FF
#define PBMAP_MIPS           0x100
#define PBMAP_MIPS_N_GPON    0x180
#define DEFAULT_PBVLAN_MAP   0x1FF
#define PBMAP_MIPS_N_EPON    0x180
#define PBMAP_UNIS           0x7F
#define PBMAP_EPON           0x80
#define MAX_EXT_SWITCH_PORTS 6     // This applies to Legacy external switches
/* ***************************************************
 * Extended Register definitions for Star Fighter2.
 *****************************************************/
#if defined(STAR_FIGHTER2)

#include "bcmmii_xtn.h"
#endif

#endif /* _BCMMII_H_ */
