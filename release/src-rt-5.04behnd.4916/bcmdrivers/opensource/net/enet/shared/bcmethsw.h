/*
 Copyright 2004-2021 Broadcom Corp. All Rights Reserved.

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

#ifndef _BCMETHSW_H_
#define _BCMETHSW_H_

#if defined(CONFIG_BCM94908) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM963138)
#define SF2_REG_SHIFT   1
#else
#define SF2_REG_SHIFT   2
#endif

typedef struct {
    // following blocks are in sf2 switch
    void *core_base_phy;
    volatile uint8_t *core_base;
    volatile uint8_t *swreg_base;
    volatile uint8_t *mdio_base;
    volatile uint8_t *fcb_base;
    volatile uint8_t *acb_base;
    volatile uint8_t *lfh_base;
    // following blocks can be in sf2 or runner based
    volatile uint8_t *xbar_ctrl;
    volatile uint8_t *qphy_ctrl;
    volatile uint8_t *sphy_ctrl;
    volatile uint8_t *xport0_clk_ctrl;
    volatile uint8_t *phy_test_ctrl;
} sw_base_t;

extern sw_base_t sw_mmap_base;

#define SWITCH_ACB_BASE     sw_mmap_base.acb_base   
#define SWITCH_BASE         sw_mmap_base.core_base 
#define SWITCH_REG_BASE     sw_mmap_base.swreg_base
#define SWITCH_PHYS_BASE    sw_mmap_base.core_base_phy
#define SWITCH_LFH_BASE     sw_mmap_base.lfh_base

#define SWITCH_CROSSBAR_REG ((volatile uint32_t *)sw_mmap_base.xbar_ctrl)

#define SWITCH_DIRECT_DATA_WR_REG   (SWITCH_REG_BASE + 0x00008UL)  
#define SWITCH_DIRECT_DATA_RD_REG   (SWITCH_REG_BASE + 0x0000cUL)

#if defined(CONFIG_BCM963178) || defined(CONFIG_BCM96756) || defined(CONFIG_BCM96765)
#define SF2_ACB_PORT0_CONFIG_REG    (SWITCH_ACB_BASE + 0x00208UL)
    #define SF2_ACB_PORT_XOFF_EN            (1<<11)
#define SF2_ACB_CONTROL_QUE_MAP_0   (SWITCH_ACB_BASE + (0x0a28 - 0x0800))
#define SF2_ACB_CONTROL_QUE_MAP_1   (SWITCH_ACB_BASE + (0x0a2c - 0x0800))
#endif

#if defined(CONFIG_BCM96756) || defined(CONFIG_BCM96765)
#define SWITCH_CORE_ARLCTL_REG_SPARE0 (SWITCH_BASE + 0x2400)
#endif

#if defined(CONFIG_BCM96765)
#define SWITCH_LFH_PORT0_CONTROL_REG    (SWITCH_LFH_BASE)
    #define LFH_XGMII_SEL                       0x20
    #define LFH_XGMII_SEL_OVRD                  0x40
#endif

#if defined(CONFIG_BCM963158)
#define SWITCH_P6_LED_CTRL_REG      (SWITCH_REG_BASE+0x0088)
    #define ETHSW_LED_CTRL_LNK_OVRD_EN           0x8000
    #define ETHSW_LED_CTRL_SPD_OVRD_EN           0x4000
    #define ETHSW_LED_CTRL_LNK_STATUS_OVRD       0x2000
    #define ETHSW_LED_CTRL_SPD_OVRD_10M          0x0000
    #define ETHSW_LED_CTRL_SPD_OVRD_100M         0x0400
    #define ETHSW_LED_CTRL_SPD_OVRD_1G           0x0800
    #define ETHSW_LED_CTRL_SPD_OVRD_2P5G         0x0c00
    #define ETHSW_LED_CTRL_LNK_SPD_MASK          0x3c00

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

#elif defined(CONFIG_BCM94908)
    #define CONFIG_3x2_CROSSBAR_SUPPORT
#elif defined(CONFIG_BCM963148)
    #define CONFIG_4x2_CROSSBAR_SUPPORT
#elif defined(CONFIG_BCM963138)
    #define CONFIG_5x3_CROSSBAR_SUPPORT
#endif

#if defined(CONFIG_BCM947622)
    #define SYSPORT0_USE_RGMII                  1
    #define SYSPORT1_USE_RGMII                  2
#endif

#if defined(CONFIG_BCM963178)
#define ETHSW_SWITCH_CTRL_REG   ((volatile uint32_t *)(SWITCH_REG_BASE+ 0x0000))
    #define ETHSW_SWITCH_CTRL_SWITCH_CLK_SEL_SHIFT  27
    #define ETHSW_SWITCH_CTRL_SWITCH_CLK_SEL_MASK   (0x3<<ETHSW_SWITCH_CTRL_SWITCH_CLK_SEL_SHIFT)
    #define ETHSW_SWITCH_CTRL_SYSPORT_CLK_SEL_SHIFT 24
    #define ETHSW_SWITCH_CTRL_SYSPORT_CLK_SEL_MASK  (0x3<<ETHSW_SWITCH_CTRL_SYSPORT_CLK_SEL_SHIFT)
    #define ETHSW_SWITCH_CTRL_P8_CLK_SEL_SHIFT      3
    #define ETHSW_SWITCH_CTRL_P8_CLK_SEL_MASK       (0x3<<ETHSW_SWITCH_CTRL_P8_CLK_SEL_SHIFT)
#endif

#if defined(CONFIG_BCM963158)||defined(CONFIG_BCM963178)
    #define PHY_TEST_CNTRL ((volatile uint32_t *)sw_mmap_base.phy_test_ctrl)

    #define ETHSW_PHY_TEST_CTRL_EN_SHIFT            0
    #define ETHSW_PHY_TEST_CTRL_EN_MASK             (0x1<<ETHSW_PHY_TEST_CTRL_EN_SHIFT)
#endif

#if defined(CONFIG_BCM963138)||defined(CONFIG_BCM963148)||defined(CONFIG_BCM94908)||defined(CONFIG_BCM963158)||defined(CONFIG_BCM963178)||defined(CONFIG_BCM963146)||defined(CONFIG_BCM94912)||defined(CONFIG_BCM96813)
    #define QPHY_CNTRL ((volatile uint32_t *)sw_mmap_base.qphy_ctrl)

    // 63146, 4912
 #if defined(CONFIG_BCM963146)||defined(CONFIG_BCM94912)||defined(CONFIG_BCM96813)
    #define ETHSW_QPHY_CTRL_REF_CLK_FREQ_SHIFT      17
    #define ETHSW_QPHY_CTRL_REF_CLK_FREQ_MASK       (0x3<<ETHSW_QPHY_CTRL_REF_CLK_FREQ_SHIFT)
    #define ETHSW_QPHY_CTRL_REF_CLK_50MHZ           (0x2<<ETHSW_QPHY_CTRL_REF_CLK_FREQ_SHIFT)
    #define ETHSW_QPHY_CTRL_RESET_SHIFT             11
    #define ETHSW_QPHY_CTRL_RESET_MASK              (0x1<<ETHSW_QPHY_CTRL_RESET_SHIFT )
    #define ETHSW_QPHY_CTRL_CK25_DIS_SHIFT          10
    #define ETHSW_QPHY_CTRL_CK25_DIS_MASK           (0x1<<ETHSW_QPHY_CTRL_CK25_DIS_SHIFT)
    #define ETHSW_QPHY_CTRL_IDDQ_GLOBAL_PWR_SHIFT   6
    #define ETHSW_QPHY_CTRL_IDDQ_GLOBAL_PWR_MASK    (0xf<<ETHSW_QPHY_CTRL_IDDQ_GLOBAL_PWR_SHIFT)
 #else
    #define ETHSW_QPHY_CTRL_RESET_SHIFT             8
    #define ETHSW_QPHY_CTRL_RESET_MASK              (0x1<<ETHSW_QPHY_CTRL_RESET_SHIFT )
    #define ETHSW_QPHY_CTRL_CK25_DIS_SHIFT          7
    #define ETHSW_QPHY_CTRL_CK25_DIS_MASK           (0x1<<ETHSW_QPHY_CTRL_CK25_DIS_SHIFT)
    #define ETHSW_QPHY_CTRL_IDDQ_GLOBAL_PWR_SHIFT   6
    #define ETHSW_QPHY_CTRL_IDDQ_GLOBAL_PWR_MASK    (0x1<<ETHSW_QPHY_CTRL_IDDQ_GLOBAL_PWR_SHIFT)
 #endif

 #if defined(CONFIG_BCM94912)
    #define ETH_PHY_TOP_XPORT0_CLK_CNTRL ((volatile uint32_t *)sw_mmap_base.xport0_clk_ctrl)
    #define ETHSW_XPORT0_CLK_CNTRL_MSBUS_CLK_SEL    (1<<0)
 #endif

    #define ETHSW_QPHY_CTRL_PHYAD_BASE_SHIFT        12
    #define ETHSW_QPHY_CTRL_PHYAD_BASE_MASK         (0x1f<<ETHSW_QPHY_CTRL_PHYAD_BASE_SHIFT)
    #define ETHSW_QPHY_CTRL_EXT_PWR_DOWN_SHIFT      1
    #define ETHSW_QPHY_CTRL_EXT_PWR_DOWN_MASK       (0xf<<ETHSW_QPHY_CTRL_EXT_PWR_DOWN_SHIFT)
    #define ETHSW_QPHY_CTRL_IDDQ_BIAS_SHIFT         0
    #define ETHSW_QPHY_CTRL_IDDQ_BIAS_MASK          (0x1<<ETHSW_QPHY_CTRL_IDDQ_BIAS_SHIFT)
#endif

#if defined(CONFIG_BCM963138)||defined(CONFIG_BCM963148)||defined(CONFIG_BCM94908)||defined(CONFIG_BCM963158)||defined(CONFIG_BCM947622)||defined(CONFIG_BCM963178)||defined(CONFIG_BCM96756)||defined(CONFIG_BCM963146)
    #define SPHY_CNTRL ((volatile uint32_t *)sw_mmap_base.sphy_ctrl)

 #if defined(CONFIG_BCM963146)
    #define ETHSW_SPHY_CTRL_REF_CLK_FREQ_SHIFT      13
    #define ETHSW_SPHY_CTRL_REF_CLK_FREQ_MASK       (3<<ETHSW_QPHY_CTRL_REF_CLK_FREQ_SHIFT)
    #define ETHSW_SPHY_CTRL_REF_CLK_50MHZ           (2<<ETHSW_QPHY_CTRL_REF_CLK_FREQ_SHIFT)
 #elif defined(CONFIG_BCM947622)
    #define ETHSW_SPHY_CTRL_PLL_CLK_SEL_SHIFT       17
    #define ETHSW_SPHY_CTRL_PLL_CLK_250_MASK        (1<<ETHSW_SPHY_CTRL_PLL_CLK_SEL_SHIFT)
 #elif defined(CONFIG_BCM96756)
    #define ETHSW_SPHY_CTRL_PLL_CLK_SEL_SHIFT       17
    #define ETHSW_SPHY_CTRL_PLL_CLK_250_MASK        (1<<ETHSW_SPHY_CTRL_PLL_CLK_SEL_SHIFT)
    #define ETHSW_SPHY_CTRL_REF_CLK_SHIFT           15
    #define ETHSW_SPHY_CTRL_REF_CLK_50MHZ           (2<<ETHSW_SPHY_CTRL_REF_CLK_SHIFT)
 #endif
    #define ETHSW_SPHY_CTRL_PHYAD_SHIFT             8
    #define ETHSW_SPHY_CTRL_PHYAD_MASK              (0x1f<<ETHSW_SPHY_CTRL_PHYAD_SHIFT)
    #define ETHSW_SPHY_CTRL_RESET_SHIFT             5
    #define ETHSW_SPHY_CTRL_RESET_MASK              (0x1<<ETHSW_SPHY_CTRL_RESET_SHIFT )
    #define ETHSW_SPHY_CTRL_CK25_DIS_SHIFT          4
    #define ETHSW_SPHY_CTRL_CK25_DIS_MASK           (0x1<<ETHSW_SPHY_CTRL_CK25_DIS_SHIFT)
    #define ETHSW_SPHY_CTRL_EXT_PWR_DOWN_SHIFT      1
    #define ETHSW_SPHY_CTRL_EXT_PWR_DOWN_MASK       (0x1<<ETHSW_SPHY_CTRL_EXT_PWR_DOWN_SHIFT)
    #define ETHSW_SPHY_CTRL_IDDQ_BIAS_SHIFT         0
    #define ETHSW_SPHY_CTRL_IDDQ_BIAS_MASK          (0x1<<ETHSW_SPHY_CTRL_IDDQ_BIAS_SHIFT)
    #define ETHSW_SPHY_CTRL_IDDQ_GLOBAL_PWR_SHIFT   3
    #define ETHSW_SPHY_CTRL_IDDQ_GLOBAL_PWR_MASK    (0x1<<ETHSW_SPHY_CTRL_IDDQ_GLOBAL_PWR_SHIFT)
#endif
        
    
#endif //_BCMETHSW_H_
