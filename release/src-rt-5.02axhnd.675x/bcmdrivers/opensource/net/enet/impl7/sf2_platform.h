/*
   <:copyright-BRCM:2015:DUAL/GPL:standard
   
      Copyright (c) 2015 Broadcom 
      All Rights Reserved
   
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

/*
 *  Created on: May/2017
 *      Author: steven.hsieh@broadcom.com
 */

#ifndef _SF2_platform_H_
#define _SF2_platform_H_

typedef struct rgmii_registers_s {
        volatile uint32 *ctrl;
        volatile uint32 *rx_clk_delay;
        volatile uint32 *pad_ctrl;
        uint32 ext_physical_port;
} rgmii_registers_t;

rgmii_registers_t rgmii_port_regs[4];   // max 4 RGMIIs in 63138

#define ADD_RGMII_REGS(r_ctrl, r_clk_delay, r_pad, n_port)  \
    {rgmii_regs->ctrl = r_ctrl; rgmii_regs->rx_clk_delay = r_clk_delay; rgmii_regs->pad_ctrl = r_pad; rgmii_regs->ext_physical_port = n_port; rgmii_regs++;}

inline void platform_init_rgmii_regs_array(void)
{
    rgmii_registers_t *rgmii_regs = rgmii_port_regs;

#if defined(CONFIG_BCM94908)
    ADD_RGMII_REGS(&ETHSW_REG->rgmii_11_ctrl, &ETHSW_REG->rgmii_11_rx_clk_delay_ctrl, &MISC->miscxMIIPadCtrl[3], 11);
#elif defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148)
    ADD_RGMII_REGS(&ETHSW_REG->rgmii_5_ctrl, &ETHSW_REG->rgmii_5_rx_clk_delay_ctrl, &MISC->miscxMIIPadCtrl[1], 5);
    ADD_RGMII_REGS(&ETHSW_REG->rgmii_7_ctrl, &ETHSW_REG->rgmii_7_rx_clk_delay_ctrl, &MISC->miscxMIIPadCtrl[2], 7);
    ADD_RGMII_REGS(&ETHSW_REG->rgmii_11_ctrl, &ETHSW_REG->rgmii_11_rx_clk_delay_ctrl, &MISC->miscxMIIPadCtrl[3], 11);
    ADD_RGMII_REGS(&ETHSW_REG->rgmii_12_ctrl, &ETHSW_REG->rgmii_12_rx_clk_delay_ctrl, &MISC->miscxMIIPadCtrl[0], 12);
#elif defined(CONFIG_BCM963158) && (CONFIG_BRCM_CHIP_REV==0x63158A0)
    ADD_RGMII_REGS(&ETHSW_REG->rgmii_11_ctrl, &ETHSW_REG->rgmii_11_rx_clk_delay_ctrl, &MISC->miscxMIIPadCtrl[1], 11);
    ADD_RGMII_REGS(&ETHSW_REG->rgmii_12_ctrl, &ETHSW_REG->rgmii_12_rx_clk_delay_ctrl, &MISC->miscxMIIPadCtrl[0], 12);
    ADD_RGMII_REGS(&ETHSW_REG->rgmii_13_ctrl, &ETHSW_REG->rgmii_13_rx_clk_delay_ctrl, &MISC->miscxMIIPadCtrl[2], 13);
#elif defined(CONFIG_BCM963158) && (CONFIG_BRCM_CHIP_REV==0x63158B0)
    ADD_RGMII_REGS(&ETHSW_REG->rgmii_11_ctrl, &ETHSW_REG->rgmii_11_rx_clk_delay_ctrl, &TOPCTRL->xMIIPadCtrl[1], 11);
    ADD_RGMII_REGS(&ETHSW_REG->rgmii_12_ctrl, &ETHSW_REG->rgmii_12_rx_clk_delay_ctrl, &TOPCTRL->xMIIPadCtrl[0], 12);
    ADD_RGMII_REGS(&ETHSW_REG->rgmii_13_ctrl, &ETHSW_REG->rgmii_13_rx_clk_delay_ctrl, &TOPCTRL->xMIIPadCtrl[2], 13);
#elif defined(CONFIG_BCM963178)
    ADD_RGMII_REGS(&ETHSW_REG->rgmii_5_ctrl, &ETHSW_REG->rgmii_5_rx_clk_delay_ctrl, &TOPCTRL->Rgmii1PadCtl, 5);
#elif defined(CONFIG_BCM947622)
    ADD_RGMII_REGS(&SYSPORT_MISC->SYSTEMPORT_MISC_RGMII_CNTRL, &SYSPORT_MISC->SYSTEMPORT_MISC_RGMII_RX_CLOCK_DELAY_CNTRL, &GPIO->PadCtrl, 10);
#endif
}

inline void platform_set_imp_speed(void)
{
#if defined(SF2_EXTERNAL)
    uint32 val32;
    // if sf2 connect thru serdes program SGMII sequences
    if (sf2_sw->s.parent_port->p.phy->phy_drv->phy_type == PHY_TYPE_SF2_SERDES) {
        // program 53134 IMP SMGII force 2.5G fiber (sequence provided by 53134 AE)
        val32 = 0x0001; SF2SW_WREG(0xe6, 0x00, &val32, 1);
        val32 = 0x8000; SF2SW_WREG(0x14, 0x3e, &val32, 2);  // BLK0 Block Address
        val32 = 0x0c2f; SF2SW_WREG(0x14, 0x20, &val32, 2);  // disable pll start sequencer
        val32 = 0x8300; SF2SW_WREG(0x14, 0x3e, &val32, 2);  // Digital Block Address
        val32 = 0x010d; SF2SW_WREG(0x14, 0x20, &val32, 2);  // enable fiber mode
        val32 = 0xc010; SF2SW_WREG(0x14, 0x30, &val32, 2);  // force 2.5G fiber enable, 50Mhz refclk

        val32 = 0x8340; SF2SW_WREG(0x14, 0x3e, &val32, 2);  // Digital5 Block Addres
        val32 = 0x0001; SF2SW_WREG(0x14, 0x34, &val32, 2);  // set os2 mode
        val32 = 0x8000; SF2SW_WREG(0x14, 0x3e, &val32, 2);  // BLK0 Block Address
        val32 = 0x0140; SF2SW_WREG(0x14, 0x00, &val32, 2);  // disable AN, set 1G mode
        val32 = 0x2c2f; SF2SW_WREG(0x14, 0x20, &val32, 2);  // enable pll start sequencer

        // override p5 & IMP port status
        val32 = 0x004a; SF2SW_WREG(PAGE_CONTROL, 0x5d, &val32, 1);  // port 5 override  no override
        val32 = 0x008b; SF2SW_WREG(PAGE_CONTROL, 0x0e, &val32, 1);  // imp port override 2.5g duplex link up
    }
    
#else //!SF2_EXTERNAL
    // based on impl5/bcmsw.c:sf2_enable_2_5g()
    volatile u32 *sw_ctrl_reg = (void *)(SWITCH_REG_BASE);
    uint32 val32 = *sw_ctrl_reg;
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM94908)
    val32 |= SF2_IMP_2_5G_EN;
#elif defined(CONFIG_BCM963158)
    val32 &= ~(IMP_SPEED_MASK);
    val32 |= DEFAULT_IMP_SPEEDS;
#endif
    *sw_ctrl_reg = val32;
#endif //!SF2_EXTERNAL
}

inline void platform_enable_p8_rdp_sel(void)
{
#if defined(CONFIG_BCM94908)
    volatile u32 *sw_ctrl_reg = (void *)(SWITCH_REG_BASE);
    uint32 val32 = *sw_ctrl_reg;
    val32 |= 1<<16; /* P8_RDP_SEL valid in 4908 */
    *sw_ctrl_reg = val32;
#elif defined(CONFIG_BCM963158)
    volatile u32 *sw_cross_bar_reg = (void*)(SWITCH_CROSSBAR_REG);
    uint32 val32 = *sw_cross_bar_reg;
 #if !defined(ARCHER_DEVICE)
    val32 |= 1<<5; /* P8_MUX_SEL 0=SysPort, 1=XRDP */
 #else
    val32 &= ~(1<<5); /* for runner disabled platforms use sysport instead of XRDP */
 #endif

    *sw_cross_bar_reg = val32;
#endif
}

inline void platform_set_clock_normal(void)
{
    uint32 reg_val32;

#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM94908)
    /* From PLL low power mode to Normal mode */
    reg_val32 = 0xc0;
    SF2SW_WREG(PAGE_CONTROL, 0xdc, &reg_val32, 4);
    reg_val32 = 0x0;
    SF2SW_WREG(PAGE_CONTROL, 0xdc, &reg_val32, 4);

    /* Bring back system clock and mac clocks */
    SF2SW_RREG(PAGE_CONTROL, REG_LOW_POWER_CTRL, &reg_val32, 4);
    reg_val32 &= ~(REG_LOW_POWER_CTR_SLEEP_P8     | REG_LOW_POWER_CTR_SLEEP_P5                 |
                   REG_LOW_POWER_CTR_SLEEP_P4     | REG_LOW_POWER_CTR_TIMER_DISABLE            |
                   REG_LOW_POWER_CTR_EN_LOW_POWER | REG_LOW_POWER_CTR_LOW_POWER_DIVIDER_6P25MHZ);
    SF2SW_WREG(PAGE_CONTROL, REG_LOW_POWER_CTRL, &reg_val32, 4);
#else
    /* change switch clock back to operation mode */
#if defined(CONFIG_BCM963158)
    pmc_switch_clock_lowpower_mode (0);
#elif defined(CONFIG_BCM963178)
    /* Return to fixed clocks */
    reg_val32 = ETHSW_REG->switch_ctrl &
      ~(ETHSW_SWITCH_CTRL_SWITCH_CLK_SEL_MASK |
        ETHSW_SWITCH_CTRL_SYSPORT_CLK_SEL_MASK |
        ETHSW_SWITCH_CTRL_P8_CLK_SEL_MASK);
    ETHSW_REG->switch_ctrl = reg_val32 |
      (2<<ETHSW_SWITCH_CTRL_SWITCH_CLK_SEL_SHIFT) |
      (2<<ETHSW_SWITCH_CTRL_SYSPORT_CLK_SEL_SHIFT) |
      (3<<ETHSW_SWITCH_CTRL_P8_CLK_SEL_SHIFT);
#endif

    /* Bring back system clock and mac clocks */
    SF2SW_RREG(PAGE_CONTROL, REG_LOW_POWER_CTRL, &reg_val32, 4);
    reg_val32 &= ~(REG_LOW_POWER_CTR_TIMER_DISABLE);
    SF2SW_WREG(PAGE_CONTROL, REG_LOW_POWER_CTRL, &reg_val32, 4);
#endif
}

inline void platform_set_clock_slow(void)
{
    uint32 reg_val32;

#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM94908)
    /* Slow down system clock, stop port5 and port 8 mac clock */
    SF2SW_RREG(PAGE_CONTROL, REG_LOW_POWER_CTRL, &reg_val32, 4);
    reg_val32 |= REG_LOW_POWER_CTR_SLEEP_P8     | REG_LOW_POWER_CTR_SLEEP_P5                 |
                 REG_LOW_POWER_CTR_SLEEP_P4     | REG_LOW_POWER_CTR_TIMER_DISABLE            |
                 REG_LOW_POWER_CTR_EN_LOW_POWER | REG_LOW_POWER_CTR_LOW_POWER_DIVIDER_6P25MHZ;
    SF2SW_WREG(PAGE_CONTROL, REG_LOW_POWER_CTRL, &reg_val32, 4);

    /* Shut Down Channel 2 PLL */
    reg_val32 = 0x1f;
    SF2SW_WREG(PAGE_CONTROL, 0xdc, &reg_val32, 4);
#else /* defined(CONFIG_BCM963158) || defined(CONFIG_BCM963158) */
    /* Slow down system clock */
    SF2SW_RREG(PAGE_CONTROL, REG_LOW_POWER_CTRL, &reg_val32, 4);
    reg_val32 |= REG_LOW_POWER_CTR_TIMER_DISABLE;
    SF2SW_WREG(PAGE_CONTROL, REG_LOW_POWER_CTRL, &reg_val32, 4);

    /* change switch clock back to lowpower mode */
#if defined(CONFIG_BCM963158)
    pmc_switch_clock_lowpower_mode (1);
#elif defined(CONFIG_BCM963178)
    /* Use preprogrammed variable speed clock */
    ETHSW_REG->switch_ctrl &=
      ~(ETHSW_SWITCH_CTRL_SWITCH_CLK_SEL_MASK |
        ETHSW_SWITCH_CTRL_SYSPORT_CLK_SEL_MASK |
        ETHSW_SWITCH_CTRL_P8_CLK_SEL_MASK);
#endif
#endif
}

// ============================================================================
#if   defined(CONFIG_BCM963138)
    #define DEFAULT_IMP_PBMAP       (PBMAP_MIPS)

// ============================================================================
#elif defined(CONFIG_BCM963148)
    #define DEFAULT_IMP_PBMAP       (PBMAP_MIPS)

// ============================================================================
#elif defined(CONFIG_BCM94908)

#if defined(CONFIG_BCM_ENET_MULTI_IMP_SUPPORT)
    #define DEFAULT_IMP_PBMAP       (PBMAP_MIPS|PBMAP_P5_IMP|PBMAP_P4_IMP)
    const int imp_to_emac[BP_MAX_SWITCH_PORTS+1] = {-1,-1,-1,-1, 2, 1,-1,-1, 0};

    /* Below mapping is used when P7 comes up at 2.5G link speed */
    int port_imp_map_2_5g[BP_MAX_SWITCH_PORTS] = 
                        {
                            /* P0 */  P4_PORT_ID,
                            /* P1 */  P5_PORT_ID,
                            /* P2 */  P5_PORT_ID,
                            /* P3 */  P5_PORT_ID, /* 5 /*/
                            /* P4 */  -1,   /* IMP Port */
                            /* P5 */  -1,   /* IMP Port */
                            /* P6 */  -1,   /* Unused/undefined switch port */
                            /* P7 */  IMP_PORT_ID /* 8 */
                        };
    /* Below mapping is used when P7 comes up at below 2.5G link speed */
    int port_imp_map_non_2_5g[BP_MAX_SWITCH_PORTS] = 
                        {
                            /* P0 */  P4_PORT_ID,
                            /* P1 */  P5_PORT_ID,
                            /* P2 */  P5_PORT_ID,
                            /* P3 */  IMP_PORT_ID, /* 8 /*/
                            /* P4 */  -1,   /* IMP Port */
                            /* P5 */  -1,   /* IMP Port */
                            /* P6 */  -1,   /* Unused/undefined switch port */
                            /* P7 */  IMP_PORT_ID /* 8 */
                        };

uint32_t imp_pbmap[BP_MAX_ENET_MACS] = {[0 ... (BP_MAX_ENET_MACS-1)] = DEFAULT_IMP_PBMAP};
static int port_imp_emac_map[BP_MAX_SWITCH_PORTS] = {[0 ... (BP_MAX_SWITCH_PORTS-1)] = -1}; 

#endif //CONFIG_BCM_ENET_MULTI_IMP_SUPPORT

// ============================================================================
#elif defined(CONFIG_BCM963158)

#if defined(CONFIG_BCM_ENET_MULTI_IMP_SUPPORT)
    #define DEFAULT_IMP_PBMAP       (PBMAP_MIPS|PBMAP_P7_IMP|PBMAP_P5_IMP)
    const int imp_to_emac[BP_MAX_SWITCH_PORTS+1] = {-1,-1,-1,-1,-1, 1,-1, 2, 0};

    int port_imp_map_2_5g[BP_MAX_SWITCH_PORTS] = 
                        {
                            /* P0 */  P5_PORT_ID,
                            /* P1 */  P5_PORT_ID,
                            /* P2 */  P5_PORT_ID,
                            /* P3 */  P7_PORT_ID,
                            /* P4 */  P7_PORT_ID,
                            /* P5 */  -1,   /* IMP Port */
                            /* P6 */  IMP_PORT_ID, /* 8 */
                            /* P7 */  -1,   /* IMP Port */
                        };

    /* for 653158, both map_2_5g and map_non_2_5g are the same */
    #define port_imp_map_non_2_5g   port_imp_map_2_5g

uint32_t imp_pbmap[BP_MAX_ENET_MACS] = {[0 ... (BP_MAX_ENET_MACS-1)] = DEFAULT_IMP_PBMAP};
static int port_imp_emac_map[BP_MAX_SWITCH_PORTS] = {[0 ... (BP_MAX_SWITCH_PORTS-1)] = -1}; 

#else
    #define DEFAULT_IMP_PBMAP       (PBMAP_P8_IMP)
#endif //CONFIG_BCM_ENET_MULTI_IMP_SUPPORT

// ============================================================================
#elif defined(CONFIG_BCM963178)
    #define DEFAULT_IMP_PBMAP       (PBMAP_MIPS)
    
    // In 63178, there are 28 tx queues given max 6 outward facing ports 5px4q+1px8q=28q
    // one port can have 8 tx queues, most likely this is WAN port.
    // Port to queue assignment can only be configured during init and can't be changed.
    // Following compile time define will set specified port to use 8Qs so no remapping.
    // Adjust this port number to match WAN port port number.
#if !defined(CONFIG_BCM_HND_EAP)
    #define PORT_WITH_8TXQ          4
#endif

// ============================================================================
#elif defined(CONFIG_BCM947622)
    #define DEFAULT_IMP_PBMAP       (PBMAP_MIPS)
#endif

#endif

