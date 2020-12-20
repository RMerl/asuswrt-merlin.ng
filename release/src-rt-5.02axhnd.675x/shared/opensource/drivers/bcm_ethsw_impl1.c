/*
 * <:copyright-BRCM:2013:DUAL/GPL:standard
 * 
 *    Copyright (c) 2013 Broadcom 
 *    All Rights Reserved
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
 */

#include "boardparms.h"

#ifdef _CFE_
#include "lib_types.h"
#include "lib_printf.h"
#include "lib_string.h"
#include "cfe_timer.h"
#include "bcm_map.h"
#define printk  printf
#define udelay cfe_usleep
#else // Linux
#include <linux/kernel.h>
#include <linux/module.h>
#include <bcm_map_part.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/sched.h>
#endif

#define _EXT_SWITCH_INIT_
#include "mii_shared.h"
#include "pmc_drv.h"
#include "pmc_switch.h"
#include "pmc_sysport.h"
#include "bcm_ethsw.h"
#include "shared_utils.h"

#include "bcm_led.h"

#define K1CTL_REPEATER_DTE 0x400
#if !defined(K1CTL_1000BT_FDX)
#define K1CTL_1000BT_FDX 	0x200
#endif
#if !defined(K1CTL_1000BT_HDX)
#define K1CTL_1000BT_HDX 	0x100
#endif

#if !defined(ARRAY_SIZE)
#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))
#endif

/*
  This file implement the switch and phy related init and low level function that can be shared between
  cfe and linux.These are functions called from CFE or from the Linux ethernet driver.

  The Linux ethernet driver handles any necessary locking so these functions should not be called
  directly from elsewhere.
*/
#if defined(__KERNEL__)
extern spinlock_t mdio_access;
#endif

#if defined(CONFIG_BCM963158) || defined(CONFIG_BCM963178) || defined(CONFIG_BCM947622) || !defined(__KERNEL__)
    // 63158 only has one mdio master, so no need to switch

#define sf2_mdio_master_enable()
#define sf2_mdio_master_disable()

#else   // !BCM963158
#define SF2_MDIO_MASTER                     0x01

static void sf2_mdio_master_enable(void)
{
    volatile uint32_t *sw_ctrl_reg = (void *)(SWITCH_REG_BASE);
    uint32_t val32 = *sw_ctrl_reg;
    
    val32 |= SF2_MDIO_MASTER;
    *sw_ctrl_reg = val32;
}

static void sf2_mdio_master_disable(void)
{
    volatile uint32_t *sw_ctrl_reg = (void *)(SWITCH_REG_BASE);
    uint32_t val32 = *sw_ctrl_reg;
    val32 &= ~SF2_MDIO_MASTER;
    *sw_ctrl_reg = val32;
}
#endif // !BCM963158

/* SF2 switch phy register access function */
uint16_t bcm_ethsw_phy_read_reg(int phy_id, int reg)
{
    int reg_in, reg_out = 0;
    int i = 0;

#if defined(__KERNEL__)
    spin_lock_bh(&mdio_access);
    sf2_mdio_master_disable();
#endif
    phy_id &= BCM_PHY_ID_M;

    reg_in = ((phy_id << ETHSW_MDIO_C22_PHY_ADDR_SHIFT)&ETHSW_MDIO_C22_PHY_ADDR_MASK) |
                ((reg << ETHSW_MDIO_C22_PHY_REG_SHIFT)&ETHSW_MDIO_C22_PHY_REG_MASK) |
                (ETHSW_MDIO_CMD_C22_READ << ETHSW_MDIO_CMD_SHIFT);
    ETHSW_MDIO->mdio_cmd = reg_in | ETHSW_MDIO_BUSY;
    do {
         if (++i >= 10)  {
             printk("%s MDIO No Response! phy 0x%x reg 0x%x \n", __FUNCTION__, phy_id, reg);
             return 0;
         }
         udelay(60);
         reg_out = ETHSW_MDIO->mdio_cmd;
    } while (reg_out & ETHSW_MDIO_BUSY);

    /* Read a second time to ensure it is reliable */
    ETHSW_MDIO->mdio_cmd = reg_in | ETHSW_MDIO_BUSY;
    i = 0;
    do {
         if (++i >= 10)  {
             printk("%s MDIO No Response! phy 0x%x reg 0x%x \n", __FUNCTION__, phy_id, reg);
             return 0;
         }
         udelay(60);
         reg_out = ETHSW_MDIO->mdio_cmd;
    } while (reg_out & ETHSW_MDIO_BUSY);

#if defined(__KERNEL__)
    sf2_mdio_master_enable();
    spin_unlock_bh(&mdio_access);
#endif
    return (uint16_t)reg_out;
}

void bcm_ethsw_phy_write_reg(int phy_id, int reg, uint16 data)
{
    int reg_value = 0;
    int i = 0;

#if defined(__KERNEL__)
    spin_lock_bh(&mdio_access);
    sf2_mdio_master_disable();
#endif
    phy_id &= BCM_PHY_ID_M;
 
    reg_value = ((phy_id << ETHSW_MDIO_C22_PHY_ADDR_SHIFT)&ETHSW_MDIO_C22_PHY_ADDR_MASK) |
                ((reg << ETHSW_MDIO_C22_PHY_REG_SHIFT)& ETHSW_MDIO_C22_PHY_REG_MASK) |
                (ETHSW_MDIO_CMD_C22_WRITE << ETHSW_MDIO_CMD_SHIFT) | (data&ETHSW_MDIO_PHY_DATA_MASK);
    ETHSW_MDIO->mdio_cmd = reg_value | ETHSW_MDIO_BUSY;

    do {
         if (++i >= 10)  {
             printk("%s MDIO No Response! phy 0x%x reg 0x%x\n", __FUNCTION__, phy_id, reg);
             return;
         }
         udelay(60);
         reg_value = ETHSW_MDIO->mdio_cmd;
    } while (reg_value & ETHSW_MDIO_BUSY);

#if defined(__KERNEL__)
    sf2_mdio_master_enable();
    spin_unlock_bh(&mdio_access);
#endif
    return;
}

#if defined(_BCM963138_) || defined(CONFIG_BCM963138) || defined(_BCM963148_) || defined(CONFIG_BCM963148) || defined(_BCM94908_) || defined(CONFIG_BCM94908) || defined(_BCM963158_) || defined(CONFIG_BCM963158) || defined(_BCM963178_) || defined(CONFIG_BCM963178) || defined(_BCM947622_) || defined(CONFIG_BCM947622)
/* EGPHY28 phy misc and expansion register indirect access function. Hard coded MII register
number, define them in bcmmii.h. Should consider move the bcmmii.h to shared folder as well */
uint16 bcm_ethsw_phy_read_exp_reg(int phy_id, int reg)
{
    bcm_ethsw_phy_write_reg(phy_id, 0x17, reg|0xf00);
    return bcm_ethsw_phy_read_reg(phy_id, 0x15);
}


void bcm_ethsw_phy_write_exp_reg(int phy_id, int reg, uint16 data)
{
    bcm_ethsw_phy_write_reg(phy_id, 0x17, reg|0xf00);
    bcm_ethsw_phy_write_reg(phy_id, 0x15, data);

    return;
}

uint16 bcm_ethsw_phy_read_misc_reg(int phy_id, int reg, int chn)
{
    uint16 temp;
    bcm_ethsw_phy_write_reg(phy_id, 0x18, 0x7);
    temp = bcm_ethsw_phy_read_reg(phy_id, 0x18);
    temp |= 0x800;
    bcm_ethsw_phy_write_reg(phy_id, 0x18, temp);

    temp = (chn << 13)|reg;
    bcm_ethsw_phy_write_reg(phy_id, 0x17, temp);
    return  bcm_ethsw_phy_read_reg(phy_id, 0x15);
}

void bcm_ethsw_phy_write_misc_reg(int phy_id, int reg, int chn, uint16 data)
{
    uint16 temp;
    bcm_ethsw_phy_write_reg(phy_id, 0x18, 0x7);
    temp = bcm_ethsw_phy_read_reg(phy_id, 0x18);
    temp |= 0x800;
    bcm_ethsw_phy_write_reg(phy_id, 0x18, temp);

    temp = (chn << 13)|reg;
    bcm_ethsw_phy_write_reg(phy_id, 0x17, temp);
    bcm_ethsw_phy_write_reg(phy_id, 0x15, data);
    
    return;
}
#endif

/* FIXME - same code exists in robosw_reg.c */
static void phy_advertise_caps(unsigned int phy_id)
{
    uint16 cap_mask = 0;

    /* control advertising if boardparms says so */
    if(IsPhyConnected(phy_id) && IsPhyAdvCapConfigValid(phy_id))
    {
        cap_mask = bcm_ethsw_phy_read_reg(phy_id, MII_ANAR);
        cap_mask &= ~(ANAR_TXFD | ANAR_TXHD | ANAR_10FD | ANAR_10HD);
        if (phy_id & ADVERTISE_10HD)
            cap_mask |= ANAR_10HD;
        if (phy_id & ADVERTISE_10FD)
            cap_mask |= ANAR_10FD;
        if (phy_id & ADVERTISE_100HD)
            cap_mask |= ANAR_TXHD;
        if (phy_id & ADVERTISE_100FD)
            cap_mask |= ANAR_TXFD;
        bcm_ethsw_phy_write_reg(phy_id, MII_ANAR, cap_mask);

        cap_mask = bcm_ethsw_phy_read_reg(phy_id, MII_K1CTL);
        cap_mask &= (~(K1CTL_1000BT_FDX | K1CTL_1000BT_HDX));
        if (phy_id & ADVERTISE_1000HD)
            cap_mask |= K1CTL_1000BT_HDX;
        if (phy_id & ADVERTISE_1000FD)
            cap_mask |= K1CTL_1000BT_FDX;
        bcm_ethsw_phy_write_reg(phy_id, MII_K1CTL, cap_mask);
    }

    /* Always enable repeater mode */
    cap_mask = bcm_ethsw_phy_read_reg(phy_id, MII_K1CTL);
    cap_mask |= K1CTL_REPEATER_DTE;
    bcm_ethsw_phy_write_reg(phy_id, MII_K1CTL, cap_mask);
}

#if defined(_BCM947622_) || defined(CONFIG_BCM947622)

uint16 phy_read_ext_bank_reg(int phy_id, int reg)
{
    uint16 bank = reg & BRCM_MIIEXT_BANK_MASK;;
    uint16 offset = (reg & BRCM_MIIEXT_OFF_MASK) + BRCM_MIIEXT_OFFSET;
    uint16 val;
    
    bcm_ethsw_phy_write_reg(phy_id, BRCM_MIIEXT_BANK, bank);
    val = bcm_ethsw_phy_read_reg(phy_id, offset);
    if (bank != BRCM_MIIEXT_DEF_BANK || offset == BRCM_MIIEXT_OFFSET)
        bcm_ethsw_phy_write_reg(phy_id, BRCM_MIIEXT_BANK, BRCM_MIIEXT_DEF_BANK);
        
    return val;
}

void phy_write_ext_bank_reg(int phy_id, int reg, uint16 data)
{
    uint16 bank = reg & BRCM_MIIEXT_BANK_MASK;;
    uint16 offset = (reg & BRCM_MIIEXT_OFF_MASK) + BRCM_MIIEXT_OFFSET;

    bcm_ethsw_phy_write_reg(phy_id, BRCM_MIIEXT_BANK, bank);
    bcm_ethsw_phy_write_reg(phy_id, offset, data);
    if (bank != BRCM_MIIEXT_DEF_BANK || offset == BRCM_MIIEXT_OFFSET)
        bcm_ethsw_phy_write_reg(phy_id, BRCM_MIIEXT_BANK, BRCM_MIIEXT_DEF_BANK);
}

//// external switch defintion & operations
#define PBMAP_MIPS 0x100

static uint32_t sw_rreg(int page, int reg, int len)
{
    uint16_t val;
    uint32_t data32 = 0;
    uint16_t *data = (uint16_t *)&data32;
    int i;

    val = (page << REG_PPM_REG16_SWITCH_PAGE_NUMBER_SHIFT) | REG_PPM_REG16_MDIO_ENABLE;
    bcm_ethsw_phy_write_reg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG16, val);

    val = (reg << REG_PPM_REG17_REG_NUMBER_SHIFT) | REG_PPM_REG17_OP_READ;
    bcm_ethsw_phy_write_reg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG17, val);

    for (i = 0; i < 20; i++) {
        val = bcm_ethsw_phy_read_reg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG17);
        if ((val & (REG_PPM_REG17_OP_WRITE | REG_PPM_REG17_OP_READ)) == REG_PPM_REG17_OP_DONE)
            break;
        udelay(10);
    }

    if (i >= 20) {
        printk("sf2_rreg: mdio timeout!\n");
        return data32;
    }

    switch (len) {
    case 1:
        val = bcm_ethsw_phy_read_reg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24);
        data[0] = (uint8_t)val; break;
    case 2:
        val = bcm_ethsw_phy_read_reg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24);
        data[0] = val; break;
    case 4:
        val = bcm_ethsw_phy_read_reg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24);
        data[0] = val;
        val = bcm_ethsw_phy_read_reg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG25);
        data[1] = val; break;
    default:
        printk("%s: len = %d NOT Handled !! \n", __FUNCTION__, len);
        break;
    }
    //printk("sf2_rreg(page=%x reg=%x len=%d %04x %04x %04x %04x)\n", page, reg, len, data[0], data[1], data[2],data[3]);
    return data32;
}

static void sw_wreg(int page, int reg, uint32_t data_in, int len)
{
    uint16_t val;
    uint16_t *data = (uint16_t *)&data_in;
    int i;

    val = (page << REG_PPM_REG16_SWITCH_PAGE_NUMBER_SHIFT) | REG_PPM_REG16_MDIO_ENABLE;
    bcm_ethsw_phy_write_reg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG16, val);

    switch (len) {
    case 1:
        val = (uint8_t)(data[0]);
        bcm_ethsw_phy_write_reg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24, val); break;
    case 2:
        bcm_ethsw_phy_write_reg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24, data[0]); break;
    case 4:
        bcm_ethsw_phy_write_reg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24, data[0]);
        bcm_ethsw_phy_write_reg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG25, data[1]); break;
    default:
        printk("%s: len = %d NOT Handled !! \n", __FUNCTION__, len);
        return;
    }

    val = (reg << REG_PPM_REG17_REG_NUMBER_SHIFT) | REG_PPM_REG17_OP_WRITE;
    bcm_ethsw_phy_write_reg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG17, val);

    for (i = 0; i < 20; i++) {
        val = bcm_ethsw_phy_read_reg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG17);
        if ((val & (REG_PPM_REG17_OP_WRITE | REG_PPM_REG17_OP_READ)) == REG_PPM_REG17_OP_DONE)
            break;
        udelay(10);
    }

    if (i >= 20)
        printk("sf2_wreg: mdio timeout!\n");
}

static int ext_sw_id = 0;
static int ext_sw_sgmii = 0;
static void sw_reset(unsigned short configType)
{
    uint32_t val;
    
    // todo: check (configType == BP_ENET_CONFIG_MDIO), now assume MDIO
    ext_sw_id = sw_rreg(PAGE_MANAGEMENT, REG_DEVICE_ID, 4);
    printk("Software Resetting Switch (Id=%x) ... ", ext_sw_id);
    val = sw_rreg(PAGE_CONTROL, SOFTWARE_RESET_CTRL, 1);
    sw_wreg(PAGE_CONTROL, SOFTWARE_RESET_CTRL, val|SOFTWARE_RESET|EN_SW_RST, 1);
    for (; sw_rreg(PAGE_CONTROL, SOFTWARE_RESET_CTRL, 1)&SOFTWARE_RESET;) udelay(100);
    printk("Done.\n");
    udelay(1000);
}

static void sw_hw_ready(void)
{
    int i;

    if (!ext_sw_id) return;
    printk("Waiting MAC port Rx/Tx to be enabled by hardware ...");
    for( i = 0; i < BP_MAX_SWITCH_PORTS; i++ )
    {
        /* Wait until hardware enable the ports, or we will kill the hardware */
        for(;sw_rreg(PAGE_CONTROL,PORT_CTRL_PORT+i,1) & PORT_CTRL_RX_DISABLE; udelay(100));
    }
}

static void sw_setup(void)
{
    int i;
    uint32_t val;

    if (!ext_sw_id) return;

    // check switch SGMII/RGMII boardparam matching 53134 strap value, if not display warning
    val = sw_rreg(PAGE_STATUS,REG_STRAP_VAL,4);
    if (val&REG_STRAP_P8_SEL_SGMII)
    {
        if (!ext_sw_sgmii) printk("\n!!!! Error: 53134 P8_SEL_SGMII is strapped high, but boardId selected is using RGMII interconnect.!!!!\n\n");
    }
    else
    {
        if (ext_sw_sgmii) printk("\n!!!! Error: 53134 P8_SEL_SGMII is strapped low, but boardId selected is using SGMII interconnect.!!!!\n\n");
    }

    printk("Disable Switch All MAC port Rx/Tx\n");
    for( i = 0; i < BP_MAX_SWITCH_PORTS; i++ )
    {
        val = sw_rreg(PAGE_CONTROL,PORT_CTRL_PORT+i,1);
        sw_wreg(PAGE_CONTROL, PORT_CTRL_PORT+i, val|PORT_CTRL_RXTX_DISABLE, 1);
    }

    /* Set switch to unmanaged mode and enable forwarding */
    val = sw_rreg(PAGE_CONTROL,REG_SWITCH_MODE,1) | REG_SWITCH_MODE_SW_FWDG_EN | REG_SWITCH_MODE_RETRY_LIMIT_DIS;
    sw_wreg(PAGE_CONTROL,REG_SWITCH_MODE, val & ~REG_SWITCH_MODE_FRAME_MANAGE_MODE, 1);
    sw_wreg(PAGE_MANAGEMENT,REG_BRCM_HDR_CTRL,0,1);
    val = sw_rreg(PAGE_CONTROL,REG_SWITCH_CONTROL,2) | REG_SWITCH_CONTROL_MII_DUMP_FWD_EN;
    sw_wreg(PAGE_CONTROL,REG_SWITCH_CONTROL,val,2);
    val = REG_CONTROL_MPSO_MII_SW_OVERRIDE|REG_CONTROL_MPSO_FLOW_CONTROL|REG_CONTROL_MPSO_SPEED1000|REG_CONTROL_MPSO_FDX|REG_CONTROL_MPSO_LINKPASS;
    sw_wreg(PAGE_CONTROL,REG_CONTROL_MII1_PORT_STATE_OVERRIDE,val,1);

    if (ext_sw_sgmii)
    {
        sw_wreg(0xe6, 0x00, 0x0001, 1);
        sw_wreg(0x14, 0x3e, 0x8000, 2);  // BLK0 Block Address
        sw_wreg(0x14, 0x20, 0x0c2f, 2);  // disable pll start sequencer
        sw_wreg(0x14, 0x3e, 0x8300, 2);  // Digital Block Address
        sw_wreg(0x14, 0x20, 0x010d, 2);  // enable fiber mode
        sw_wreg(0x14, 0x30, 0xc010, 2);  // force 2.5G fiber enable, 50Mhz refclk

        sw_wreg(0x14, 0x3e, 0x8340, 2);  // Digital5 Block Addres
        sw_wreg(0x14, 0x34, 0x0001, 2);  // set os2 mode
        sw_wreg(0x14, 0x3e, 0x8000, 2);  // BLK0 Block Address
        sw_wreg(0x14, 0x00, 0x0140, 2);  // disable AN, set 1G mode
        sw_wreg(0x14, 0x20, 0x2c2f, 2);  // enable pll start sequencer

        sw_wreg(PAGE_CONTROL, 0x5d, 0x004a, 1); // port 5 override  no override
        sw_wreg(PAGE_CONTROL, 0x0e, 0x008b, 1); // imp port override 2.5g duplex link up
    }
}

static int saved = 0; 
static uint32_t portCtrl[BP_MAX_SWITCH_PORTS], pbvlan[BP_MAX_SWITCH_PORTS];

static void sw_reg_save(void)
{
    int i;

    if (saved) return;
    saved = 1;
    for( i = 0; i < BP_MAX_SWITCH_PORTS; i++ )
    {
        portCtrl[i] = sw_rreg(PAGE_CONTROL,PORT_CTRL_PORT+i,1);
        pbvlan[i] = sw_rreg(PAGE_PORT_BASED_VLAN,REG_VLAN_CTRL_P0+i*2,2);
    }
}

static void sw_reg_restore(void)
{
    int i;
    uint32_t val;

    if (!saved) return;
    for( i = 0; i < BP_MAX_SWITCH_PORTS; i++ )
    {
        val = sw_rreg(PAGE_CONTROL,PORT_CTRL_PORT+i,1) & PORT_CTRL_SWITCH_RESERVE;
        val |= portCtrl[i] & ~PORT_CTRL_SWITCH_RESERVE;
        sw_wreg(PAGE_CONTROL,PORT_CTRL_PORT+i,val,1);
        sw_wreg(PAGE_PORT_BASED_VLAN,REG_VLAN_CTRL_P0+i*2, pbvlan[i],2);
    }
}

static void sw_open(void)
{
    int i;
    uint32_t val;

    if (!ext_sw_id) return;
    sw_reg_save();

    printk ("Enable Switch MAC Port Rx/Tx, set PBVLAN to FAN out, set switch to NO-STP.\n");
    for( i = 0; i < BP_MAX_SWITCH_PORTS; i++ )
    {
        /* Set Port VLAN to allow CPU traffic only */
        sw_wreg(PAGE_PORT_BASED_VLAN,REG_VLAN_CTRL_P0+i*2,PBMAP_MIPS,2);

        /* Setting switch to NO-STP mode; enable port TX/RX. */
        val = sw_rreg(PAGE_CONTROL,PORT_CTRL_PORT+i,1) & (~(PORT_CTRL_RXTX_DISABLE|PORT_CTRL_PORT_STATUS_M));
        sw_wreg(PAGE_CONTROL,PORT_CTRL_PORT+i,val|PORT_CTRL_NO_STP,1);
    }
}

static void sw_close(void)
{
    if (!ext_sw_id) return;
    sw_reg_restore();
}

static void phy_rgmii_enable_ib_st(unsigned int phy_id)
{
    // based on BCM54210 phy
    uint16 val;
    phy_id &= BCM_PHY_ID_M;
    val = 0x7007;   // select MII_REG_18_SHADOW_MISC_CTRL to read
    bcm_ethsw_phy_write_reg(phy_id, MII_AUXCTL, val);
    val = bcm_ethsw_phy_read_reg(phy_id, MII_AUXCTL);
    val &= ~(1<<5); // clear RGMII out-of-band status disable
    val |= 0x8000;  // enable write
    bcm_ethsw_phy_write_reg(phy_id, MII_AUXCTL, val);
}

static void phy_rgmii_setup(int unit, int port, unsigned int phy_id, unsigned int flags)
{
    volatile uint32 *rgmii_ctrl = &(SYSPORT_MISC->SYSTEMPORT_MISC_RGMII_CNTRL);
    volatile uint32 *rgmii_rx_clk_delay = &(SYSPORT_MISC->SYSTEMPORT_MISC_RGMII_RX_CLOCK_DELAY_CNTRL);
    uint32 rgmii_ctrl_v, rgmii_rx_clk_delay_v;

    rgmii_ctrl_v = (*rgmii_ctrl & ~ETHSW_RC_MII_MODE_MASK) | ETHSW_RC_RGMII_EN | ETHSW_RC_ID_MODE_DIS;
    rgmii_ctrl_v |= ETHSW_RC_EXT_GPHY;

    rgmii_rx_clk_delay_v = *rgmii_rx_clk_delay;

    if (IsPortTxInternalDelay(flags))
        rgmii_ctrl_v &= ~ETHSW_RC_ID_MODE_DIS; /* Clear TX_ID_DIS */
    if (IsPortRxInternalDelay(flags))
        rgmii_rx_clk_delay_v &= ~(ETHSW_RXCLK_IDDQ|ETHSW_RXCLK_BYPASS); /* Clear Rx bypass */

    *rgmii_rx_clk_delay = rgmii_rx_clk_delay_v;
    *rgmii_ctrl = rgmii_ctrl_v;

    // configure crossbar for RGMII connection
    SYSPORT_MISC->SYSTEMPORT_MISC_CROSSBAR3X2_CONTROL = (port) ? SYSPORT1_USE_RGMII : SYSPORT0_USE_RGMII;

    // enable external phy in band link speed status
    if (IsExtPhyId(phy_id))
        phy_rgmii_enable_ib_st(phy_id);

    printk("sysport%d - RGMII - %s InternalDelay:(Tx-%d Rx-%d) ...", port,
        IsRGMII_1P8V(phy_id)? "1.8v":IsRGMII_2P5V(phy_id)? "2.5v":"3.3v",
        IsPortTxInternalDelay(flags), IsPortRxInternalDelay(flags));
}

static uint16 serdesRef50mVco6p25 [] =
{
    0x8000, 0x0c2f,
    0x8308, 0xc000,
    0x8050, 0x5740,
    0x8051, 0x01d0,
    0x8052, 0x19f0,
    0x8053, 0xaab0,
    0x8054, 0x8821,
    0x8055, 0x0044,
    0x8056, 0x8000,
    0x8057, 0x0872,
    0x8058, 0x0000,

    0x8106, 0x0020,
    0x8054, 0x8021,
    0x8054, 0x8821,
};

static uint16 serdesSet2p5GFiber [] =
{
    0x0010, 0x0C2F,       /* disable pll start sequencer */
    0x8066, 0x0009,       /* Set AFE for 2.5G */
    0x8065, 0x1620,       
    0x8300, 0x0149,       /* enable fiber mode, also depend board parameters */
    0x8308, 0xC010,       /* Force 2.5G Fiber, enable 50MHz refclk */
    0x834a, 0x0001,       /* Set os2 mode */
    0x0000, 0x0140,       /* disable AN, set 1G mode */
    0x0010, 0x2C2F,       /* enable pll start sequencer */
};

static void config_serdes(int phy_addr, uint16 seq[], int seqSize)
{
    int i;
    seqSize /= sizeof(seq[0]);
    for (i=0; i<seqSize; i+=2)
        if (seq[i] < 0x20)  // CL22 space
            bcm_ethsw_phy_write_reg(phy_addr, seq[i], seq[i+1]);
        else
            phy_write_ext_bank_reg(phy_addr, seq[i], seq[i+1]);
}

static void phy_sgmii_init(void)
{
    uint32 val32 = *(uint32 *)SWITCH_REG_SINGLE_SERDES_CNTRL;
    int phy_addr = ext_sw_sgmii & BCM_PHY_ID_M;
    
    val32 |= SWITCH_REG_SERDES_RESETPLL|SWITCH_REG_SERDES_RESETMDIO|SWITCH_REG_SERDES_RESET;
    val32 &= ~(SWITCH_REG_SERDES_IDDQ|SWITCH_REG_SERDES_PWRDWN);
    *(uint32 *)SWITCH_REG_SINGLE_SERDES_CNTRL = val32;
    udelay(1000);
    val32 &= ~(SWITCH_REG_SERDES_RESETPLL|SWITCH_REG_SERDES_RESETMDIO|SWITCH_REG_SERDES_RESET);
    *(uint32 *)SWITCH_REG_SINGLE_SERDES_CNTRL = val32;
    udelay(1000);

    // do dummy MDIO read to workaround ASIC problem
    bcm_ethsw_phy_read_reg(phy_addr, 0);

    config_serdes(phy_addr, serdesRef50mVco6p25, sizeof(serdesRef50mVco6p25));
    udelay(1000);
    // serdesSet2p5GFiber
    config_serdes(phy_addr, serdesSet2p5GFiber, sizeof(serdesSet2p5GFiber));
}

static void phy_sgmii_setup(int unit, int port, unsigned int phy_id)
{
    ext_sw_sgmii = phy_id;
    printk("sysport%d - SGMII ...", port);
}

#endif //47622

#if defined(_BCM963138_) || defined(CONFIG_BCM963138)
static void phy_adjust_afe(unsigned int phy_id_base, int is_quad)
{
    unsigned int phy_id;
    unsigned int phy_id_end = is_quad ? (phy_id_base + 4) : (phy_id_base + 1);

    for( phy_id = phy_id_base; phy_id < phy_id_end; phy_id++ )
    {
        //reset phy
        bcm_ethsw_phy_write_reg(phy_id, 0x0, 0x9140);
        udelay(100);

        //AFE_RXCONFIG_1 Provide more margin for INL/DNL measurement on ATE  
        bcm_ethsw_phy_write_misc_reg(phy_id, 0x38, 0x1, 0x9b2f);
        //AFE_TX_CONFIG Set 100BT Cfeed=011 to improve rise/fall time
        bcm_ethsw_phy_write_misc_reg(phy_id, 0x39, 0x0, 0x0431);
        //AFE_VDAC_ICTRL_0 Set Iq=1101 instead of 0111 for improving AB symmetry 
        bcm_ethsw_phy_write_misc_reg(phy_id, 0x39, 0x1, 0xa7da);
        //AFE_HPF_TRIM_OTHERS Set 100Tx/10BT to -4.5% swing & Set rCal offset for HT=0 code
        bcm_ethsw_phy_write_misc_reg(phy_id, 0x3a, 0x0, 0x00e3);
    }

    //CORE_BASE1E Force trim overwrite and set I_ext trim to 0000
    bcm_ethsw_phy_write_reg(phy_id_base, 0x1e, 0x10);
    for( phy_id = phy_id_base; phy_id < phy_id_end; phy_id++ )
    {
        //Adjust bias current trim by +4% swing, +2 tick 'DSP_TAP10
        bcm_ethsw_phy_write_misc_reg(phy_id, 0xa, 0x0, 0x011b);
    }

    //Reset R_CAL/RC_CAL Engine 'CORE_EXPB0
    bcm_ethsw_phy_write_exp_reg(phy_id_base, 0xb0, 0x10);
    //Disable Reset R_CAL/RC_CAL Engine 'CORE_EXPB0
    bcm_ethsw_phy_write_exp_reg(phy_id_base, 0xb0, 0x0);

    return;
}

#endif

#if defined(_BCM963148_) || defined(CONFIG_BCM963148)
static void phy_adjust_afe(unsigned int phy_id_base, int is_quad)
{
    unsigned int phy_id;
    unsigned int phy_id_end = is_quad ? (phy_id_base + 4) : (phy_id_base + 1);

    for( phy_id = phy_id_base; phy_id < phy_id_end; phy_id++ )
    {
        //reset phy
        bcm_ethsw_phy_write_reg(phy_id, 0x0, 0x9140);
        udelay(100);
        //Write analog control registers
        //AFE_RXCONFIG_0
        bcm_ethsw_phy_write_misc_reg(phy_id, 0x38, 0x0, 0xeb15);
        //AFE_RXCONFIG_1. Replacing the previously suggested 0x9AAF for SS part. See JIRA HW63148-31
        bcm_ethsw_phy_write_misc_reg(phy_id, 0x38, 0x1, 0x9b2f);
        //AFE_RXCONFIG_2
        bcm_ethsw_phy_write_misc_reg(phy_id, 0x38, 0x2, 0x2003);
        //AFE_RX_LP_COUNTER
        bcm_ethsw_phy_write_misc_reg(phy_id, 0x38, 0x3, 0x7fc0);
        //AFE_TX_CONFIG
        bcm_ethsw_phy_write_misc_reg(phy_id, 0x39, 0x0, 0x0060);
        //AFE_VDAC_ICTRL_0
        bcm_ethsw_phy_write_misc_reg(phy_id, 0x39, 0x1, 0xa7da);
        //AFE_VDAC_OTHERS_0
        bcm_ethsw_phy_write_misc_reg(phy_id, 0x39, 0x3, 0xa020);
        //AFE_HPF_TRIM_OTHERS
        bcm_ethsw_phy_write_misc_reg(phy_id, 0x3a, 0x0, 0x00e3);
    }

    //CORE_BASE1E Force trim overwrite and set I_ext trim to 0000
    bcm_ethsw_phy_write_reg(phy_id_base, 0x1e, 0x0010);
    for( phy_id = phy_id_base; phy_id < phy_id_end; phy_id++ )
    {
       //Adjust bias current trim by +4% swing, +2 tick, increase PLL BW in GPHY link start up training 'DSP_TAP10
       bcm_ethsw_phy_write_misc_reg(phy_id, 0xa, 0x0, 0x111b);
    }

    //Reset R_CAL/RC_CAL Engine 'CORE_EXPB0
    bcm_ethsw_phy_write_exp_reg(phy_id_base, 0xb0, 0x10);
    //Disable Reset R_CAL/RC_CAL Engine 'CORE_EXPB0
    bcm_ethsw_phy_write_exp_reg(phy_id_base, 0xb0, 0x0);
}

#endif

#if defined(_BCM94908_) || defined(CONFIG_BCM94908)
static void phy_adjust_afe(unsigned int phy_id_base, int is_quad)
{
    unsigned int phy_id;
    unsigned int phy_id_end = is_quad ? (phy_id_base + 4) : (phy_id_base + 1);

    for( phy_id = phy_id_base; phy_id < phy_id_end; phy_id++ )
    {
        //reset phy
        bcm_ethsw_phy_write_reg(phy_id, 0x0, 0x9140);
    }
    udelay(100);

    //CORE_BASE1E Force trim overwrite and set I_ext trim to 0000
    bcm_ethsw_phy_write_reg(phy_id_base, 0x1e, 0x0010);
    for( phy_id = phy_id_base; phy_id < phy_id_end; phy_id++ )
    {
        //AFE_RXCONFIG_1 Provide more margin for INL/DNL measurement on ATE  
        bcm_ethsw_phy_write_misc_reg(phy_id, 0x38, 0x1, 0x9b2f);
        //AFE_TX_CONFIG Set 1000BT Cfeed=011 to improve rise/fall time
        bcm_ethsw_phy_write_misc_reg(phy_id, 0x39, 0x0, 0x0431);
        //AFE_VDAC_ICTRL_0 Set Iq=1101 instead of 0111 for improving AB symmetry 
        bcm_ethsw_phy_write_misc_reg(phy_id, 0x39, 0x1, 0xa7da);
        //AFE_HPF_TRIM_OTHERS Set 100Tx/10BT to -4.5% swing & Set rCal offset for HT=0 code
        bcm_ethsw_phy_write_misc_reg(phy_id, 0x3a, 0x0, 0x00e3);
        //DSP_TAP10  Adjust I_int bias current trim by +0% swing, +0 tick
        bcm_ethsw_phy_write_misc_reg(phy_id, 0xa, 0x0, 0x011b);
    }

    //Reset R_CAL/RC_CAL Engine 'CORE_EXPB0
    bcm_ethsw_phy_write_exp_reg(phy_id_base, 0xb0, 0x10);
    //Disable Reset R_CAL/RC_CAL Engine 'CORE_EXPB0
    bcm_ethsw_phy_write_exp_reg(phy_id_base, 0xb0, 0x0);

    return;
}
#endif

#if defined(_BCM963158_) || defined(CONFIG_BCM963158) || \
    defined(_BCM963178_) || defined(CONFIG_BCM963178) || \
    defined(_BCM947622_) || defined(CONFIG_BCM947622)
static void phy_adjust_afe(unsigned int phy_id_base, int is_quad)
{
    unsigned int phy_id;
    unsigned int phy_id_end = is_quad ? (phy_id_base + 4) : (phy_id_base + 1);

    for( phy_id = phy_id_base; phy_id < phy_id_end; phy_id++ )
    {
        //reset phy
        bcm_ethsw_phy_write_reg(phy_id, 0x0, 0x9140);
    }
    udelay(100);


    for( phy_id = phy_id_base; phy_id < phy_id_end; phy_id++ )
    {
        //Turn off AOF
        bcm_ethsw_phy_write_misc_reg(  phy_id,  0x39, 0x1, 0x0000 );//AFE_TX_CONFIG_0

        //1g AB symmetry Iq
        bcm_ethsw_phy_write_misc_reg(  phy_id,  0x3a, 0x2, 0x0BCC );//AFE_TX_CONFIG_1

        //LPF BW
        bcm_ethsw_phy_write_misc_reg(  phy_id,  0x39, 0x0, 0x233F );//AFE_TX_IQ_RX_LP

        //RCAL +6LSB to make impedance from 112 to 100ohm
        bcm_ethsw_phy_write_misc_reg(  phy_id,  0x3b, 0x0, 0xAD40 );//AFE_TEMPSEN_OTHERS

        //since rcal make R smaller, make master current -4%
        bcm_ethsw_phy_write_misc_reg(  phy_id,  0x0a, 0x0, 0x091B );//DSP_TAP10

        //From EEE excel config file for Vitesse fix
        bcm_ethsw_phy_write_misc_reg(  phy_id,  0x0021, 0x0002, 0x87F6 );// rx_on_tune 8 -> 0xf
        bcm_ethsw_phy_write_misc_reg(  phy_id,  0x0022, 0x0002, 0x017D );// 100tx EEE bandwidth
        bcm_ethsw_phy_write_misc_reg(  phy_id,  0x0026, 0x0002, 0x0015 );// enable ffe zero det for Vitesse interop

    }

    //Reset R_CAL/RC_CAL Engine 'CORE_EXPB0
    bcm_ethsw_phy_write_exp_reg(phy_id_base, 0xb0, 0x10);
    //Disable Reset R_CAL/RC_CAL Engine 'CORE_EXPB0
    bcm_ethsw_phy_write_exp_reg(phy_id_base, 0xb0, 0x0);

    return;

}
#endif

static void phy_fixup(void)
{
    int phy_id;

    /* Internal QUAD PHY and Single PHY require some addition fixup on the PHY AFE */
#if defined(QPHY_CNTRL)
    phy_id = (*QPHY_CNTRL &ETHSW_QPHY_CTRL_PHYAD_BASE_MASK)>>ETHSW_QPHY_CTRL_PHYAD_BASE_SHIFT;
    phy_adjust_afe(phy_id, 1);
#endif
    phy_id = (*SPHY_CNTRL &ETHSW_SPHY_CTRL_PHYAD_MASK)>>ETHSW_SPHY_CTRL_PHYAD_SHIFT;
    phy_adjust_afe(phy_id, 0);
}

#if defined(ETHSW_CORE)
static void extsw_register_save_restore(int save)
{
    static int saved = 0; 
    static uint32_t portCtrl[BP_MAX_SWITCH_PORTS], pbvlan[BP_MAX_SWITCH_PORTS], reg;
    int i;
    int offset_jump=ARRAY_SIZE(ETHSW_CORE->port_vlan_ctrl)/9;

    if (save) {
        saved = 1;
        for( i = 0; i < BP_MAX_SWITCH_PORTS; i++ )
        {
            portCtrl[i] = ETHSW_CORE->port_traffic_ctrl[i] & 0xff;
            pbvlan[i] = ETHSW_CORE->port_vlan_ctrl[offset_jump*i] & 0xffff;
        }
    }
    else
    {
        if (saved)
        {
            for( i = 0; i < BP_MAX_SWITCH_PORTS; i++ )
            {
                reg = ETHSW_CORE->port_traffic_ctrl[i] & PORT_CTRL_SWITCH_RESERVE;
                reg |= portCtrl[i] & ~PORT_CTRL_SWITCH_RESERVE;
                ETHSW_CORE->port_traffic_ctrl[i] = reg;
                ETHSW_CORE->port_vlan_ctrl[offset_jump*i] = pbvlan[i];
            }
        }
    }
}
#endif //ETHSW_CORE

#if defined(CONFIG_BCM963158) || defined(_BCM963158_) || \
    defined(CONFIG_BCM963178) || defined(_BCM963178_) || \
    defined(CONFIG_BCM947622) || defined(_BCM947622_)

#if defined(CONFIG_BCM963178) || defined(_BCM963178_) || \
    defined(CONFIG_BCM947622) || defined(_BCM947622_)
#define GPHY_INIT_POWER_DELAY()                                         \
    do {                                                                \
        int _i;                                                         \
        for(_i=0; _i<13; ++_i) {                                        \
            udelay(1000*2);                                             \
        }                                                               \
    } while(0)
#else
#define GPHY_INIT_POWER_DELAY()                                         \
    do {                                                                \
        udelay(1000*15);udelay(1000*10);                                \
    } while(0)
#endif

void all_gphy_init_power_workaround(void)
{

    unsigned int phy_ctrl;

#if defined(QPHY_CNTRL)
    *QPHY_CNTRL |= ETHSW_QPHY_CTRL_RESET_MASK;
#endif
    *SPHY_CNTRL |= ETHSW_SPHY_CTRL_RESET_MASK;
    GPHY_INIT_POWER_DELAY();

    *PHY_TEST_CTRL =1;

#if defined(QPHY_CNTRL)
    phy_ctrl = *QPHY_CNTRL;
    phy_ctrl &= ~(ETHSW_QPHY_CTRL_IDDQ_BIAS_MASK|ETHSW_QPHY_CTRL_IDDQ_GLOBAL_PWR_MASK);
    *QPHY_CNTRL = phy_ctrl;
#endif
    phy_ctrl = *SPHY_CNTRL;
    phy_ctrl &= ~(ETHSW_SPHY_CTRL_IDDQ_BIAS_MASK|ETHSW_SPHY_CTRL_IDDQ_GLOBAL_PWR_MASK);
    *SPHY_CNTRL = phy_ctrl;
    GPHY_INIT_POWER_DELAY();
    
#if defined(QPHY_CNTRL)
    *QPHY_CNTRL |= (ETHSW_QPHY_CTRL_IDDQ_BIAS_MASK|ETHSW_QPHY_CTRL_IDDQ_GLOBAL_PWR_MASK);
#endif
    *SPHY_CNTRL |= (ETHSW_SPHY_CTRL_IDDQ_BIAS_MASK|ETHSW_SPHY_CTRL_IDDQ_GLOBAL_PWR_MASK);
    GPHY_INIT_POWER_DELAY();

#if defined(QPHY_CNTRL)
    phy_ctrl = *QPHY_CNTRL;
    phy_ctrl &= ~(ETHSW_QPHY_CTRL_RESET_MASK);
    *QPHY_CNTRL = phy_ctrl;
#endif
    phy_ctrl = *SPHY_CNTRL;
    phy_ctrl &= ~(ETHSW_SPHY_CTRL_RESET_MASK);
    *SPHY_CNTRL = phy_ctrl;
    GPHY_INIT_POWER_DELAY();

    *PHY_TEST_CTRL =0;
}
#endif


/* SF2 switch low level init */
void bcm_ethsw_init(void)
{
    const ETHERNET_MAC_INFO   *pMacInfo = BpGetEthernetMacInfoArrayPtr();
    unsigned int phy_ctrl;
    unsigned short phy_base;
    int i, sw;

    printk("Initalizing switch low level hardware.\n");
#if defined(ETHSW_CORE)
    /* power up the switch block */
    pmc_switch_power_up();
#endif

#if defined(_BCM947622_) || defined(CONFIG_BCM947622)
    pmc_sysport_power_up();
#endif
    /* power up unimac for CFE */
#if defined(_CFE_) && defined(_BCM94908_)
    PowerOnDevice(PMB_ADDR_GMAC);
#endif

#if defined(ETHSW_CORE)
    /* Reset switch */
    printk("Software Resetting Switch ... ");
    ETHSW_CORE->software_reset |= SOFTWARE_RESET|EN_SW_RST;
    for (;ETHSW_CORE->software_reset & SOFTWARE_RESET;) udelay(100);
    printk("Done.\n");
    udelay(1000);
#elif defined(_BCM947622_) || defined(CONFIG_BCM947622)
    if (pMacInfo[1].ucPhyType == BP_ENET_EXTERNAL_SWITCH)
        sw_reset(pMacInfo[1].usConfigType);
#endif

    if( BpGetGphyBaseAddress(&phy_base) != BP_SUCCESS )
        phy_base = 1;

#if defined(CONFIG_BCM963158) || defined(_BCM963158_) || \
    defined(CONFIG_BCM963178) || defined(_BCM963178_) || \
    defined(CONFIG_BCM947622) || defined(_BCM947622_)
    all_gphy_init_power_workaround();
#endif
    /* power on and reset the quad and single phy */
#if defined(QPHY_CNTRL)
    phy_ctrl = *QPHY_CNTRL;
    phy_ctrl &= ~(ETHSW_QPHY_CTRL_IDDQ_BIAS_MASK|ETHSW_QPHY_CTRL_EXT_PWR_DOWN_MASK|ETHSW_QPHY_CTRL_PHYAD_BASE_MASK);
    phy_ctrl |= ETHSW_QPHY_CTRL_RESET_MASK|(phy_base<<ETHSW_QPHY_CTRL_PHYAD_BASE_SHIFT);
#if defined(ETHSW_QPHY_CTRL_IDDQ_GLOBAL_PWR_MASK)
    phy_ctrl &= ~(ETHSW_QPHY_CTRL_IDDQ_GLOBAL_PWR_MASK);
#endif

    *QPHY_CNTRL = phy_ctrl;
#endif //QPHY_CNTRL

    phy_ctrl = *SPHY_CNTRL;
    phy_ctrl &= ~(ETHSW_SPHY_CTRL_IDDQ_BIAS_MASK| ETHSW_SPHY_CTRL_EXT_PWR_DOWN_MASK|ETHSW_SPHY_CTRL_PHYAD_MASK);
#if defined(QPHY_CNTRL)
    phy_ctrl |= ETHSW_SPHY_CTRL_RESET_MASK|((phy_base+4)<<ETHSW_SPHY_CTRL_PHYAD_SHIFT);
#else
    phy_ctrl |= ETHSW_SPHY_CTRL_RESET_MASK|(phy_base<<ETHSW_SPHY_CTRL_PHYAD_SHIFT);
#endif

#if defined(ETHSW_SPHY_CTRL_IDDQ_GLOBAL_PWR_MASK)
    phy_ctrl &= ~(ETHSW_SPHY_CTRL_IDDQ_GLOBAL_PWR_MASK);
#endif

    *SPHY_CNTRL = phy_ctrl;

    udelay(1000);
#if defined(QPHY_CNTRL)
    *QPHY_CNTRL &= ~ETHSW_QPHY_CTRL_RESET_MASK;
#endif
    *SPHY_CNTRL &= ~ETHSW_SPHY_CTRL_RESET_MASK;

    udelay(1000);


    /* add dummy read to workaround first MDIO read/write issue after power on */
    bcm_ethsw_phy_read_reg(phy_base, 0x2);

    phy_fixup();

    bcm_ethsw_led_init();

#if defined(ETHSW_CORE)
    printk("Waiting MAC port Rx/Tx to be enabled by hardware ...");
    for( i = 0; i < BP_MAX_SWITCH_PORTS; i++ )
    {
        /* Wait until hardware enable the ports, or we will kill the hardware */
        for(;ETHSW_CORE->port_traffic_ctrl[i] & PORT_CTRL_RX_DISABLE; udelay(100));
    }
#elif defined(_BCM947622_) || defined(CONFIG_BCM947622)
#if defined(RTAX95Q) || defined(RTAX56U) || defined(RTAX56_XD4) || defined(CTAX56_XD4) || defined(RTAX55) || defined(RTAX1800)
{
#define SWITCH_REG_MDIO_CFG         0x80411304
    printk("Change SYSTEM PORT MDIO clock to 2.5MHz\n");
    *(uint32 *)SWITCH_REG_MDIO_CFG = 0x00002311;
    udelay(1000);
}
#endif
    sw_hw_ready();
#endif

    for (sw = 0 ; sw < BP_MAX_ENET_MACS ; sw++) 
    {
        for(i = 0; i < BP_MAX_SWITCH_PORTS; i++ )
        {
            unsigned int phy_id = pMacInfo[sw].sw.phy_id[i];
            phy_advertise_caps(phy_id);
#if defined(_BCM947622_) || defined(CONFIG_BCM947622)
            if (IsRGMII(phy_id))
                phy_rgmii_setup(sw, i, phy_id, pMacInfo[sw].sw.port_flags[i]);
            else if (IsSerdes(phy_id) && IsPortConnectedToExternalSwitch(phy_id))
                phy_sgmii_setup(sw, i, phy_id);
#endif
        }
        for (i = 0; i < BP_MAX_CROSSBAR_EXT_PORTS ; i++ )
        {
            const ETHERNET_CROSSBAR_INFO *crossbar = &(pMacInfo[sw].sw.crossbar[i]);
            if (crossbar->switch_port != BP_CROSSBAR_NOT_DEFINED)
            {
                phy_advertise_caps(crossbar->phy_id);
#if defined(_BCM947622_) || defined(CONFIG_BCM947622)
                if (IsRGMII(crossbar->phy_id))
                    phy_rgmii_setup(sw, crossbar->switch_port, crossbar->phy_id, crossbar->port_flags);
#endif
            }
        }
    }
    printk("Done\n");

#if defined(ETHSW_CORE)
    /* disabled all MAC TX/RX. */
    printk("Disable Switch All MAC port Rx/Tx\n");
    for( i = 0; i < BP_MAX_SWITCH_PORTS; i++ )
    {
        ETHSW_CORE->port_traffic_ctrl[i] = (ETHSW_CORE->port_traffic_ctrl[i] & 0xff) | PORT_CTRL_RXTX_DISABLE;
    }

    /* Set switch to unmanaged mode and enable forwarding */
    ETHSW_CORE->switch_mode = ((ETHSW_CORE->switch_mode & 0xff) | ETHSW_SM_FORWARDING_EN |
            ETHSW_SM_RETRY_LIMIT_DIS) & (~ETHSW_SM_MANAGED_MODE);
    ETHSW_CORE->brcm_hdr_ctrl = 0;
    ETHSW_CORE->switch_ctrl = (ETHSW_CORE->switch_ctrl & 0xffb0) | 
        ETHSW_SC_MII_DUMP_FORWARDING_EN | ETHSW_SC_MII2_VOL_SEL;

    ETHSW_CORE->imp_port_state = ETHSW_IPS_USE_REG_CONTENTS | ETHSW_IPS_TXFLOW_PAUSE_CAPABLE | 
        ETHSW_IPS_RXFLOW_PAUSE_CAPABLE | ETHSW_IPS_SW_PORT_SPEED_1000M_2000M | 
        ETHSW_IPS_DUPLEX_MODE | ETHSW_IPS_LINK_PASS;
#elif defined(_BCM947622_) || defined(CONFIG_BCM947622)
    sw_setup();
#endif

    return;
}

#if defined(_BCM94908_)

/* In 4908 GMAC/UNIMAC is attached to the SF2 using MAC to MAC connection as IMP port. There is no
   PHY block. Sneak in some simple GMAC init routine in the driver file. 
   TODO: Move to seperate file for sharing with linux driver */
static void gmac_init(void);
static void gmac_enable_port(int enable);

static void gmac_init(void)
{
    /* Reset GMAC */
    GMAC_MAC->Cmd.sw_reset = 1;
    cfe_usleep(20);
    GMAC_MAC->Cmd.sw_reset = 0;
   
    GMAC_INTF->Flush.txfifo_flush = 1;
    GMAC_INTF->Flush.rxfifo_flush = 1;

    GMAC_INTF->Flush.txfifo_flush = 0;
    GMAC_INTF->Flush.rxfifo_flush = 0;

    GMAC_INTF->MibCtrl.clrMib = 1;
    GMAC_INTF->MibCtrl.clrMib = 0;

    /* default CMD configuration */
    GMAC_MAC->Cmd.eth_speed = CMD_ETH_SPEED_1000;

    /* Disable Tx and Rx */
    GMAC_MAC->Cmd.tx_ena = 0;
    GMAC_MAC->Cmd.rx_ena = 0;

    GMAC_INTF->GmacStatus.auto_cfg_en = 1;
    GMAC_INTF->GmacStatus.hd = 0;
    GMAC_INTF->GmacStatus.eth_speed = 2;
    GMAC_INTF->GmacStatus.link_up = 1;

    return;
}

static void gmac_enable_port(int enable)
{
    if( enable )
    {
        GMAC_MAC->Cmd.tx_ena = 1;
        GMAC_MAC->Cmd.rx_ena = 1;
    }
    else
    {
        GMAC_MAC->Cmd.tx_ena = 0;
        GMAC_MAC->Cmd.rx_ena = 0;
    }

    return;
}
#endif

/* SF2 switch init for CFE networking */
/* Only Called by CFE Command Line */
void bcm_ethsw_open(void)
{
#if defined(ETHSW_CORE)
    int i;
    int offset_jump=ARRAY_SIZE(ETHSW_CORE->port_vlan_ctrl)/9;

    /* Save MAC port and PBVLAN registers to save boot strap status */
    extsw_register_save_restore(1);

    /* Enable MAC port Tx/Rx for CFE local traffic */
    printk ("Enable Switch MAC Port Rx/Tx, set PBVLAN to FAN out, set switch to NO-STP.\n");
    for( i = 0; i < BP_MAX_SWITCH_PORTS; i++ )
    {
        /* Set Port VLAN to allow CPU traffic only */
        ETHSW_CORE->port_vlan_ctrl[offset_jump*i] = PBMAP_MIPS;

        /* Setting switch to NO-STP mode; enable port TX/RX. */
        ETHSW_CORE->port_traffic_ctrl[i] = ((ETHSW_CORE->port_traffic_ctrl[i] & 0xff) & 
                (~(PORT_CTRL_RXTX_DISABLE|PORT_CTRL_PORT_STATUS_M))) | PORT_CTRL_NO_STP;
    }
#elif defined(_BCM947622_) || defined(CONFIG_BCM947622)
    sw_open();
    if (ext_sw_sgmii) phy_sgmii_init();
#endif

#if defined(_BCM94908_)
    gmac_init();
    gmac_enable_port(1);
#endif
    return;
}

/* SF2 switch post process to restore switch status too boot strap status */
void bcm_ethsw_close(void)
{
    printk("Restore Switch's MAC port Rx/Tx, PBVLAN back.\n");
#if defined(_BCM94908_)
    gmac_enable_port(0);
#endif
#if defined(ETHSW_CORE)
    extsw_register_save_restore(0);
#elif defined(_BCM947622_) || defined(CONFIG_BCM947622)
    sw_close();
#endif
}

#if !defined(_CFE_)
EXPORT_SYMBOL(bcm_ethsw_phy_read_reg);
EXPORT_SYMBOL(bcm_ethsw_phy_write_reg);
#endif // !_CFE_


