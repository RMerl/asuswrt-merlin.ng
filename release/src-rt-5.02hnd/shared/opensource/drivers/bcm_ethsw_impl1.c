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

#ifdef GTAC5300
#define _EXT_SWITCH_INIT_
#endif

#include "mii_shared.h"
#include "pmc_drv.h"
#include "pmc_switch.h"
#include "bcm_ethsw.h"
#include "shared_utils.h"
#include "bcm_gpio.h"
#include "robosw_reg.h"

#define K1CTL_REPEATER_DTE 0x400
#if !defined(K1CTL_1000BT_FDX)
#define K1CTL_1000BT_FDX 	0x200
#endif
#if !defined(K1CTL_1000BT_HDX)
#define K1CTL_1000BT_HDX 	0x100
#endif

/*
  This file implement the switch and phy related init and low level function that can be shared between
  cfe and linux.These are functions called from CFE or from the Linux ethernet driver.

  The Linux ethernet driver handles any necessary locking so these functions should not be called
  directly from elsewhere.
*/

/* SF2 switch phy register access function */
uint16_t bcm_ethsw_phy_read_reg(int phy_id, int reg)
{
    int reg_in, reg_out = 0;
    int i = 0;

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

    return (uint16_t)reg_out;
}

void bcm_ethsw_phy_write_reg(int phy_id, int reg, uint16 data)
{
    int reg_value = 0;
    int i = 0;

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

    return;
}

#if defined(_BCM963138_) || defined(CONFIG_BCM963138) || defined(_BCM963148_) || defined(CONFIG_BCM963148) || defined(_BCM94908_) || defined(CONFIG_BCM94908)
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

static void bcm_ethsw_pmdio_read_reg(int page, int reg, uint8 *data, int len)
{
    uint16 v;
    int i;

    v = (page << REG_PPM_REG16_SWITCH_PAGE_NUMBER_SHIFT) | REG_PPM_REG16_MDIO_ENABLE;
    bcm_ethsw_phy_write_reg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG16, v);

    v = (reg << REG_PPM_REG17_REG_NUMBER_SHIFT) | REG_PPM_REG17_OP_READ;
    bcm_ethsw_phy_write_reg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG17, v);

    for (i = 0; i < 20; i++) {
        v = bcm_ethsw_phy_read_reg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG17);
        if ((v & (REG_PPM_REG17_OP_WRITE | REG_PPM_REG17_OP_READ)) == REG_PPM_REG17_OP_DONE)
            break;
        udelay(10);
    }

    if (i >= 20) {
        printk("bcmsw_mdio_rreg: timeout!\n");
        return;
    }

    switch (len) {
         case 8:
            v = bcm_ethsw_phy_read_reg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG27);
            data[7] = (v >> 8);
            data[6] = (v & 0xff);
         case 6:
            v = bcm_ethsw_phy_read_reg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG26);
            data[5] = (v >> 8);
            data[4] = (v & 0xff);
            v = bcm_ethsw_phy_read_reg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG25);
            data[3] = (v >> 8);
            data[2] = (v & 0xff);
            v = bcm_ethsw_phy_read_reg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24);
            data[1] = (v >> 8);
            data[0] = (v & 0xff);
            break;
         case 4:
            v = bcm_ethsw_phy_read_reg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG25);
            *(uint32 *)data = (((uint32)v) << 16);
            v = bcm_ethsw_phy_read_reg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24);
            *(uint32 *)data |= v;
            break;
        case 2:
            v = bcm_ethsw_phy_read_reg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24);
            ((uint16 *)data)[0] = (uint16)v;
            break;
        case 1:
            v = bcm_ethsw_phy_read_reg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24);
            data[0] = (uint8)v;
            break;
    }
}

static void bcm_ethsw_pmdio_write_reg(int page, int reg, uint8 *data, int len)
{
    uint16 v;
    int i;

    v = (page << REG_PPM_REG16_SWITCH_PAGE_NUMBER_SHIFT) | REG_PPM_REG16_MDIO_ENABLE;
    bcm_ethsw_phy_write_reg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG16, v);

    switch (len) {
        case 8:
            v = data[7];
            v = ((v << 8) | data[6]);
            bcm_ethsw_phy_write_reg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG27, v);
        case 6:
            v = data[5];
            v = ((v << 8) | data[4]);
            bcm_ethsw_phy_write_reg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG26, v);
            v = data[3];
            v = ((v << 8) | data[2]);
            bcm_ethsw_phy_write_reg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG25, v);
            v = data[1];
            v = ((v << 8) | data[0]);
            bcm_ethsw_phy_write_reg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24, v);
            break;
        case 4:
            v = (uint16)((*(uint32 *)data) >> 16);
            bcm_ethsw_phy_write_reg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG25, v);
            v  = (uint16)(*(uint32 *)data);
            bcm_ethsw_phy_write_reg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24, v);
            break;
        case 2:
            v = *(uint16 *)data;
            bcm_ethsw_phy_write_reg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24, v);
            break;
        case 1:
            v = data[0];
            bcm_ethsw_phy_write_reg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG24, v);
            break;
    }

    v = (reg << REG_PPM_REG17_REG_NUMBER_SHIFT) | REG_PPM_REG17_OP_WRITE;
    bcm_ethsw_phy_write_reg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG17, v);

    for (i = 0; i < 20; i++) {
        v = bcm_ethsw_phy_read_reg(PSEUDO_PHY_ADDR, REG_PSEUDO_PHY_MII_REG17);
        if ((v & (REG_PPM_REG17_OP_WRITE | REG_PPM_REG17_OP_READ)) == REG_PPM_REG17_OP_DONE)
            break;
        udelay(10);
    }

    if (i >= 20)
        printk("ethsw_mdio_wreg: timeout!\n");
}

void bcm_ethsw_rreg_ext(int access_type, int page, int reg, uint8 *data, int len)
{
    if (access_type == MDIO_BUS) {
        bcm_ethsw_pmdio_read_reg(page, reg, data, len);

    } else {
        printk("%s: not support!\n", __FUNCTION__);
    }
}

void bcm_ethsw_wreg_ext(int access_type, int page, int reg, uint8 *data, int len)
{
    if (access_type == MDIO_BUS) {
        bcm_ethsw_pmdio_write_reg(page, reg, data, len);
    } else {
        printk("%s: not support!\n", __FUNCTION__);
    }
}

static void bcm_ethsw_set_led_reg(volatile uint32_t* ledctrl)
{
    uint32_t value;

    value = *ledctrl;

    /* turn off all the leds */
    value |= ETHSW_LED_CTRL_ALL_SPEED_MASK;
    
    /* broadcom reference design alway use LED_SPD0 for 1g link and LED_SPD1 for 100m link */
    value &= ~(ETHSW_LED_CTRL_SPEED_MASK << ETHSW_LED_CTRL_1000M_SHIFT);
    value |= (ETHSW_LED_CTRL_SPD0_ON << ETHSW_LED_CTRL_1000M_SHIFT)|(ETHSW_LED_CTRL_SPD1_OFF << ETHSW_LED_CTRL_1000M_SHIFT);

    value &= ~(ETHSW_LED_CTRL_SPEED_MASK<<ETHSW_LED_CTRL_100M_SHIFT);
    value |= (ETHSW_LED_CTRL_SPD0_OFF << ETHSW_LED_CTRL_100M_SHIFT)|(ETHSW_LED_CTRL_SPD1_ON << ETHSW_LED_CTRL_100M_SHIFT);

    *ledctrl = value;

    return;
}

static void bcm_ethsw_set_led(void)
{
    volatile uint32_t* ledctrl;
    uint16_t lnkLed, actLed;
    int i;

    /* set the sw internal phy LED mode. the default speed mode encoding is wrong.
       apply to all 5 internal GPHY */
    for( i = 0; i < 5; i++ )
    {
#if defined(_BCM94908_) || defined(CONFIG_BCM94908)
        ledctrl = &ETHSW_REG->led_ctrl[i].led_encoding;
#else
        ledctrl = &ETHSW_REG->led_ctrl[i];
#endif
        bcm_ethsw_set_led_reg(ledctrl);
    }

    /* WAN led */
#if defined(_BCM94908_) || defined(CONFIG_BCM94908)
    ledctrl = &ETHSW_REG->led_wan_ctrl.led_encoding;
#else
    ledctrl = &ETHSW_REG->led_wan_ctrl;
#endif
    bcm_ethsw_set_led_reg(ledctrl);

    /* aggregate LED setting */
    if (BpGetAggregateLnkLedGpio(&lnkLed) == BP_SUCCESS ||
        BpGetAggregateActLedGpio(&actLed) == BP_SUCCESS ) 
    {
#if defined(_BCM94908_) || defined(CONFIG_BCM94908)
        /* link led polarity is reversed from hw.  Suppose to be active low but it is active high.
           change the polarity in led ctrl registr and also enable all 5 GPHY ports */ 
        ETHSW_REG->aggregate_led_ctrl |= (ETHSW_AGGREGATE_LED_CTRL_LNK_POL_SEL_MASK|0xf); 
#endif
    }

    return;
}

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

static void phy_fixup(void)
{
    int phy_id;

    /* Internal QUAD PHY and Single PHY require some addition fixup on the PHY AFE */
    phy_id = (ETHSW_REG->qphy_ctrl&ETHSW_QPHY_CTRL_PHYAD_BASE_MASK)>>ETHSW_QPHY_CTRL_PHYAD_BASE_SHIFT;
    phy_adjust_afe(phy_id, 1);

    phy_id = (ETHSW_REG->sphy_ctrl&ETHSW_SPHY_CTRL_PHYAD_MASK)>>ETHSW_SPHY_CTRL_PHYAD_SHIFT;
    phy_adjust_afe(phy_id, 0);
}

#ifdef _EXT_SWITCH_INIT_
#if defined(_BCM94908_) || defined(CONFIG_BCM94908)
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
#define CB_PHY_PORT_MASK                    0x3
#define CB_PHY_PORT_SHIFT                   0x2
#define CB_PHY_MODE_RGMII                   0x2
#endif /* _BCM94908_ || CONFIG_BCM94908 */

static void crossbar_port_rgmii_config(int unit, const ETHERNET_CROSSBAR_INFO *crossbar)
{
#if defined(_BCM94908_) || defined(CONFIG_BCM94908)
    uint32 val32;
    int int_cb_port; /* internal crossbar port */

    /* config crossbar */
    if (unit == 0)
        int_cb_port = 1;
    else
        int_cb_port = 0;

    val32 = ETHSW_REG->crossbar_switch_ctrl;
    val32 &= ~(CB_PHY_PORT_MASK << (int_cb_port * CB_PHY_PORT_SHIFT));
    val32 |= ((CB_PHY_MODE_RGMII & CB_PHY_PORT_MASK) << (int_cb_port * CB_PHY_PORT_SHIFT));
    ETHSW_REG->crossbar_switch_ctrl = val32;

    /* set pad controls for 1.8v operation. */
    if (IsRGMII_1P8V(crossbar->phy_id)) {
        MISC->miscxMIIPadCtrl[3] = 0xf;
    }

    val32 = ETHSW_REG->rgmii_11_ctrl;
    val32 &= ~SF2_RGMII_PORT_MODE_M; /* Clear Mode bits defaults and set based on interface type */
    val32 |= SF2_ENABLE_PORT_RGMII_INTF ; /* Enable the (R)(G)MII mode */
    val32 |= (SF2_RGMII_PORT_MODE_EXT_GPHY_RGMII<<SF2_RGMII_PORT_MODE_S); /* Set port mode as RGMII */
    if (IsPortTxInternalDelay(crossbar->port_flags))
        val32 &= ~SF2_TX_ID_DIS;  /* Clear TX_ID_DIS */
    else
        val32 |= SF2_TX_ID_DIS; /* Disable the RGMII Internal Delay */
    ETHSW_REG->rgmii_11_ctrl = val32;
    if (IsPortRxInternalDelay(crossbar->port_flags)) {
        /* Clear Rx bypass */
        val32 = ETHSW_REG->rgmii_11_rx_clk_delay_ctrl;
        val32 &= ~(SF2_RGMII_RX_CLK_IDDQ|SF2_RX_ID_BYPASS);
        ETHSW_REG->rgmii_11_rx_clk_delay_ctrl = val32;
    }
#endif /* _BCM94908_ || CONFIG_BCM94908 */
}

static void crossbar_ext_switch_init(const ETHERNET_CROSSBAR_INFO *crossbar)
{
#define BCM53134S 0x5075
#define BCM53134_REG_IMP_RGMII_DELAY 0x60
  #define RGMII_TXCLK_DELAY   (1 << 0)
  #define RGMII_RXCLK_DELAY   (1 << 1)
#define BCM53134_REG_SW_CTRL 0x22
  #define MII_DUMB_FWDG_EN    (1 << 6)

    unsigned short resetGpio;
    int access_type;
    uint8 data8;
    uint32 data32;
    uint16 data16;

    /* reset external switch */
    resetGpio = crossbar->phyReset;
    if (resetGpio != BP_GPIO_NONE) {
        /* set gpio dir to output */
        bcm_gpio_set_dir(resetGpio, 1);
        /* hold reset */
        if (resetGpio & BP_ACTIVE_LOW)
            bcm_gpio_set_data((resetGpio & BP_GPIO_NUM_MASK), 0);
        else
            bcm_gpio_set_data((resetGpio & BP_GPIO_NUM_MASK), 1);
#ifdef _CFE_
        udelay(10000);
#else
	mdelay(10);
#endif
        /* release reset */
        if (resetGpio & BP_ACTIVE_LOW)
            bcm_gpio_set_data((resetGpio & BP_GPIO_NUM_MASK), 1);
        else
            bcm_gpio_set_data((resetGpio & BP_GPIO_NUM_MASK), 0);
        udelay(5000);
    }

    /* reg access type */
    access_type = MDIO_BUS;
    bcm_ethsw_rreg_ext(access_type, PAGE_MANAGEMENT, REG_DEVICE_ID,
        (uint8 *)&data32, sizeof(data32));
    printk("External switch id = %x \n", data32);

    if (data32 == BCM53134S) {
        /* disable IMP Port */
        data8 = 0;
        bcm_ethsw_wreg_ext(access_type, PAGE_MANAGEMENT, 
            REG_GLOBAL_CONFIG, &data8, sizeof(data8));

        /* enable unmanged mode */
	bcm_ethsw_rreg_ext(access_type, PAGE_CONTROL,
            BCM53134_REG_SW_CTRL, (uint8 *)&data16, sizeof(data8));
        data16 |= MII_DUMB_FWDG_EN;
        bcm_ethsw_wreg_ext(access_type, PAGE_CONTROL, 
            BCM53134_REG_SW_CTRL, (uint8 *)&data16, sizeof(data16));

        /* RGMII delay clk */
        bcm_ethsw_rreg_ext(access_type, PAGE_CONTROL,
            BCM53134_REG_IMP_RGMII_DELAY, &data8, sizeof(data8));
        data8 |= (RGMII_TXCLK_DELAY | RGMII_RXCLK_DELAY);
        bcm_ethsw_wreg_ext(access_type, PAGE_CONTROL,
            BCM53134_REG_IMP_RGMII_DELAY, &data8, sizeof(data8));
   }
}
#endif /* _EXT_SWITCH_INIT_ */

static void extsw_register_save_restore(int save)
{
    static int saved = 0; 
    static uint32_t portCtrl[BP_MAX_SWITCH_PORTS], pbvlan[BP_MAX_SWITCH_PORTS], reg;
    int i;
    const ETHERNET_MAC_INFO *EnetInfo = BpGetEthernetMacInfoArrayPtr();

    if (save) {
        saved = 1;
        for( i = 0; i < BP_MAX_SWITCH_PORTS; i++ )
        {
            if ((EnetInfo[1].sw.port_map & (1 << i)) == 0)
                continue;
            portCtrl[i] = ETHSW_CORE->port_traffic_ctrl[i] & 0xff;
            pbvlan[i] = ETHSW_CORE->port_vlan_ctrl[2*i] & 0xffff;
        }
    }
    else
    {
        if (saved)
        {
            for( i = 0; i < BP_MAX_SWITCH_PORTS; i++ )
            {
                if ((EnetInfo[1].sw.port_map & (1 << i)) == 0)
                    continue;

                reg = ETHSW_CORE->port_traffic_ctrl[i] & PORT_CTRL_SWITCH_RESERVE;
                reg |= portCtrl[i] & ~PORT_CTRL_SWITCH_RESERVE;
                ETHSW_CORE->port_traffic_ctrl[i] = reg;
                ETHSW_CORE->port_vlan_ctrl[2*i] = pbvlan[i];
            }
        }
    }
}

/* SF2 switch low level init */
void bcm_ethsw_init(void)
{
    const ETHERNET_MAC_INFO   *pMacInfo = BpGetEthernetMacInfoArrayPtr();
    unsigned int phy_ctrl, port_map = pMacInfo[1].sw.port_map;
    unsigned short phy_base;
    int i;

    printk("Initalizing switch low level hardware.\n");
    /* power up the switch block */
    pmc_switch_power_up();

    /* power up unimac for CFE */
#if defined(_CFE_) && defined(_BCM94908_)
    PowerOnDevice(PMB_ADDR_GMAC);
#endif

    /* Reset switch */
    printk("Software Resetting Switch ... ");
    ETHSW_CORE->software_reset |= SOFTWARE_RESET|EN_SW_RST;
    for (;ETHSW_CORE->software_reset & SOFTWARE_RESET;) udelay(100);
    printk("Done.\n");
    udelay(1000);

    if( BpGetGphyBaseAddress(&phy_base) != BP_SUCCESS )
        phy_base = 1;

    /* power on and reset the quad and single phy */
    phy_ctrl = ETHSW_REG->qphy_ctrl;
    phy_ctrl &= ~(ETHSW_QPHY_CTRL_IDDQ_BIAS_MASK|ETHSW_QPHY_CTRL_EXT_PWR_DOWN_MASK|ETHSW_QPHY_CTRL_PHYAD_BASE_MASK);
    phy_ctrl |= ETHSW_QPHY_CTRL_RESET_MASK|(phy_base<<ETHSW_QPHY_CTRL_PHYAD_BASE_SHIFT);

#if defined(_BCM94908_) || defined(CONFIG_BCM94908)
    phy_ctrl &= ~(ETHSW_QPHY_CTRL_IDDQ_GLOBAL_PWR_MASK);
#endif

    ETHSW_REG->qphy_ctrl = phy_ctrl;

    phy_ctrl = ETHSW_REG->sphy_ctrl;
    phy_ctrl &= ~(ETHSW_SPHY_CTRL_IDDQ_BIAS_MASK| ETHSW_SPHY_CTRL_EXT_PWR_DOWN_MASK|ETHSW_SPHY_CTRL_PHYAD_MASK);
    phy_ctrl |= ETHSW_SPHY_CTRL_RESET_MASK|((phy_base+4)<<ETHSW_SPHY_CTRL_PHYAD_SHIFT);

#if defined(_BCM94908_) || defined(CONFIG_BCM94908)
    phy_ctrl &= ~(ETHSW_SPHY_CTRL_IDDQ_GLOBAL_PWR_MASK);
#endif

    ETHSW_REG->sphy_ctrl = phy_ctrl;

    udelay(1000);

    ETHSW_REG->qphy_ctrl &= ~ETHSW_QPHY_CTRL_RESET_MASK;
    ETHSW_REG->sphy_ctrl &= ~ETHSW_SPHY_CTRL_RESET_MASK;

    udelay(1000);

    /* add dummy read to workaround first MDIO read/write issue after power on */
    bcm_ethsw_phy_read_reg(phy_base, 0x2);

    phy_fixup();

    bcm_ethsw_set_led();

    printk("Waiting MAC port Rx/Tx to be enabled by hardware ...");
    for( i = 0; i < BP_MAX_SWITCH_PORTS; i++ )
    {
        /* Wait until hardware enable the ports, or we will kill the hardware */
        for(;ETHSW_CORE->port_traffic_ctrl[i] & PORT_CTRL_RX_DISABLE; udelay(100));
        phy_advertise_caps(pMacInfo->sw.phy_id[i]);
    }
    printk("Done\n");

    /* disabled all MAC TX/RX. */
    printk("Disable Switch All MAC port Rx/Tx\n");
    for( i = 0; i < BP_MAX_SWITCH_PORTS; i++ )
    {
        if( ((1 << i) & port_map) == 0 ) continue;
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

#ifdef _EXT_SWITCH_INIT_
{
    int crossbar_port;
    int unit;
    int phy_port;
    const ETHERNET_MAC_INFO   *pMacInfo_ext;

    for (unit=0; unit < BP_MAX_ENET_MACS; unit++) {
        pMacInfo_ext = &pMacInfo[unit];
        for(phy_port = BP_CROSSBAR_PORT_BASE; phy_port < BP_MAX_PHY_PORTS; phy_port++) {
            crossbar_port = BP_PHY_PORT_TO_CROSSBAR_PORT(phy_port);
            if (!BP_IS_CROSSBAR_PORT_DEFINED(pMacInfo_ext->sw, crossbar_port))
                continue;

            /* RGMII config for all crossbar ports */
            if (IsRGMII(pMacInfo_ext->sw.crossbar[crossbar_port].phy_id))
                crossbar_port_rgmii_config(unit, &pMacInfo_ext->sw.crossbar[crossbar_port]);

            /* external switch init */
            if (pMacInfo_ext->sw.crossbar[crossbar_port].phyconn == PHY_CONN_TYPE_EXT_SW)
                crossbar_ext_switch_init(&pMacInfo_ext->sw.crossbar[crossbar_port]);
        }
    }
}
#endif /* _EXT_SWITCH_INIT_ */

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
    const ETHERNET_MAC_INFO   *pMacInfo = BpGetEthernetMacInfoArrayPtr();
    unsigned int port_map = pMacInfo[1].sw.port_map;
    int i;

    /* Save MAC port and PBVLAN registers to save boot strap status */
    extsw_register_save_restore(1);

    /* Enable MAC port Tx/Rx for CFE local traffic */
    printk ("Enable Switch MAC Port Rx/Tx, set PBVLAN to FAN out, set switch to NO-STP.\n");
    for( i = 0; i < BP_MAX_SWITCH_PORTS; i++ )
    {
        if( ((1 << i) & port_map) == 0 ) continue;

        /* Set Port VLAN to allow CPU traffic only */
        ETHSW_CORE->port_vlan_ctrl[2*i] = PBMAP_MIPS;

        /* Setting switch to NO-STP mode; enable port TX/RX. */
        ETHSW_CORE->port_traffic_ctrl[i] = ((ETHSW_CORE->port_traffic_ctrl[i] & 0xff) & 
                (~(PORT_CTRL_RXTX_DISABLE|PORT_CTRL_PORT_STATUS_M))) | PORT_CTRL_NO_STP;
    }

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
    extsw_register_save_restore(0);
}



