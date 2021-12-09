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

/*
 *  Created on: Dec 2016
 *      Author: steven.hsieh@broadcom.com
 */

/*
 * Phy drivers for 63138, 63148, 4908
 */
#ifndef _UBOOT_
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/module.h>
#endif
#include "bus_drv.h"
#include "phy_drv_sf2.h"
#include "phy_drv_mii.h"
#include "phy_drv_brcm.h"
#include "bcm_map_part.h"
#include "bcmnet.h"
#include "board.h"
#include "bcm_otp.h"
#include "bcm_OS_Deps.h"
#include "pmc_drv.h"
#include "bcmmii.h"
#include "bcmmii_xtn.h"
#ifndef _UBOOT_
#include "bcmsfp_i2c.h"
#endif
#include "opticaldet.h"
#include "bcm_map_part.h"
#include "bcmnet.h"
#include "board.h"
#include "bcm_otp.h"
#include "pmc_drv.h"



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

    #define MII_CONTROL_SPEED_MASK  (MII_CONTROL_SPEED_SEL13|MII_CONTROL_SPEED_SEL6)
    #define MII_CONTROL_SPEED_1000  (0<<13|1<<6)
    #define MII_CONTROL_SPEED_100   (1<<13|0<<6)
    #define MII_CONTROL_SPEED_10    (0)

#define MII_STATUS                          0x1
    #define MII_STATUS_LINK                 (1<<2)

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
    #define SHADOW_18_FORCE_MDIX_AUTO       (1<<9)

#define SERDES_1000X_CONTROL        0x8300
    #define SERDES_SIGNAL_DETECTE_EN    (1<<2)
    #define SERDES_INVERT_SIGNAL_DET    (1<<3)


DEFINE_MUTEX(bcm_phy_exp_mutex);

int ethsw_phy_exp_rw(phy_dev_t *phy_dev, u32 reg, u16 *v16_p, int rd)
{
    u32 bank, offset;
    int rc = 0;

    if (reg < 0x20) {   /* CL22 space */
        if (IsC45Phy(phy_dev)) {
            printk("phy_id: %d does not support Clause 22\n", phy_dev->addr);
            return rc;
        }
        if (rd)
            rc = phy_bus_read(phy_dev, reg, v16_p);
        else
            rc = phy_bus_write(phy_dev, reg, *v16_p);
        return rc;
    }
    else if (reg < 0x10000) /* expanded MDIO space */
    {
        bank = reg & BRCM_MIIEXT_BANK_MASK;
        offset = (reg & BRCM_MIIEXT_OFF_MASK) + BRCM_MIIEXT_OFFSET;
        mutex_lock(&bcm_phy_exp_mutex);
        /* Set Bank Address */
        rc = phy_bus_write(phy_dev, BRCM_MIIEXT_BANK, bank);

        if (!rd)
            rc += phy_bus_write(phy_dev, offset, *v16_p);
        else
            rc += phy_bus_read(phy_dev, offset, v16_p);

        /* Set Bank back to default for standard access */
        if(bank != BRCM_MIIEXT_DEF_BANK || offset == BRCM_MIIEXT_OFFSET)
            rc += phy_bus_write(phy_dev, BRCM_MIIEXT_BANK, BRCM_MIIEXT_DEF_BANK);
        mutex_unlock(&bcm_phy_exp_mutex);
    }
    else if (reg < 0x200000) /* CL45 space */
    {
        if (!IsC45Phy(phy_dev)) {
            printk("phy_id=%d does not support Clause 45\n", phy_dev->addr);
            return rc;
        }

        if (rd)
            rc = phy_bus_c45_read32(phy_dev, reg, v16_p);
        else
            rc = phy_bus_c45_write32(phy_dev, reg, *v16_p);
    }

    return rc;
}
EXPORT_SYMBOL(ethsw_phy_exp_rw);

/* Enable forced MDIX auto detection */
static void ethsw_phy_force_mdix_auto(phy_dev_t *phy_dev)
{
    u16 v16;

    phy_bus_write(phy_dev, MII_REGISTER_18, MII_REG_18_SEL(MII_REG_18_SHADOW_MISC_CTRL));
    phy_bus_read(phy_dev, MII_REGISTER_18, &v16);
    v16 |= SHADOW_18_FORCE_MDIX_AUTO;
    phy_bus_write(phy_dev, MII_REGISTER_18, MII_REG_18_WR(MII_REG_18_SHADOW_MISC_CTRL, v16));
    phy_bus_write(phy_dev, MII_REGISTER_18, 0x7007);
    phy_bus_read(phy_dev, MII_REGISTER_18, &v16);

}

static int ethsw_phy_reset(phy_dev_t *phy_dev)
{
    u16 v16;
    u32 reg;
    int i=0;

    /* Reset PHY to clear status first */

    if (IsC45Phy(phy_dev))
    {
        reg = CL45_REG_IEEE_CTRL;
        v16= MII_CONTROL_RESET;
        phy_bus_c45_write32(phy_dev, reg, v16);
        for(phy_bus_c45_read32(phy_dev, reg, &v16); v16 & MII_CONTROL_RESET;
                phy_bus_c45_read32(phy_dev, reg, &v16));
        {
            if (++i > 20) {printk("Failed to reset 0x%x\n", phy_dev->addr); return 0;}
            msleep(100);
        }
    }
    else
    {
        reg = MII_CONTROL;
        v16= MII_CONTROL_RESET;
        phy_bus_write(phy_dev, reg, v16);
        for(phy_bus_read(phy_dev, reg, &v16); v16 & MII_CONTROL_RESET;
                phy_bus_read(phy_dev, reg, &v16))
        {
            if (++i > 20) {printk("Failed to reset 0x%x\n", phy_dev->addr); return 0;}
            msleep(100);
        }

        if (phy_dev->phy_drv->phy_type == PHY_TYPE_SF2_GPHY)
            ethsw_phy_force_mdix_auto(phy_dev);
    }

    return 1;
}

static int phy_init (phy_dev_t *phy_dev)
{
    /* 
        Reset External GPHY; 
        Internal has been configured by CFE with AFE, must ont reset
    */
    if (IsExtPhyId(phy_dev->meta_id) && phy_dev->phy_drv->phy_type == PHY_TYPE_SF2_GPHY)
        ethsw_phy_reset(phy_dev);
    else {
        if (phy_dev->phy_drv->phy_type == PHY_TYPE_SF2_GPHY)
            ethsw_phy_force_mdix_auto(phy_dev);
    }

    if (0 && phy_dev_cable_diag_is_supported(phy_dev))
        phy_dev_cable_diag_set(phy_dev, 1);

    return 0;
}

static int _phy_read_status(phy_dev_t *phy_dev)
{
#if defined(CONFIG_BCM947622)
    if (IsRGMII(phy_dev->meta_id))      // update RGMII_IB_STATUS
    {
        int ret = brcm_read_status(phy_dev);
        u32 v32 = 0;

        if (phy_dev->link)
        {
            v32 |= RGMII_IB_ST_OVRD | RGMII_IB_ST_LINK_UP;
            v32 |= (phy_dev->duplex == PHY_DUPLEX_FULL)? RGMII_IB_ST_FD:0;
            switch (phy_dev->speed) {
                case PHY_SPEED_1000:    v32 |= RGMII_IB_ST_1000; break;
                case PHY_SPEED_100:     v32 |= RGMII_IB_ST_100; break;
                default:
                    printk("Unknown speed: %d\n", phy_dev->speed);
                    return -1;
            }
            
        }
        SYSPORT_MISC->SYSTEMPORT_MISC_RGMII_IB_STATUS = v32;
        return ret;
    }
    else
#endif
        return brcm_read_status(phy_dev);
}

#if defined(QPHY_CNTRL)
#if !defined(ETHSW_QPHY_CTRL_IDDQ_GLOBAL_PWR_MASK)
#define ETHSW_QPHY_CTRL_IDDQ_GLOBAL_PWR_MASK 0
#endif
static void qphy_ctrl_adjust(uint32_t ext_pwr_down)
{
    uint32_t phy_ctrl;

    if (ext_pwr_down == 0)
        return;

    phy_ctrl = *QPHY_CNTRL;
    phy_ctrl |= ext_pwr_down << ETHSW_QPHY_CTRL_EXT_PWR_DOWN_SHIFT;
    if (ext_pwr_down == 0xf)
        phy_ctrl |= ETHSW_QPHY_CTRL_CK25_DIS_MASK |
                    ETHSW_QPHY_CTRL_IDDQ_GLOBAL_PWR_MASK |
                    ETHSW_QPHY_CTRL_IDDQ_BIAS_MASK;
    *QPHY_CNTRL = phy_ctrl;
    printk("Adjusted SF2 QGPHY: qphy_ctrl=0x%08x ext_pwr_down=0x%x\n",
                phy_ctrl, ext_pwr_down);
}
#endif

#if !defined(ETHSW_SPHY_CTRL_IDDQ_GLOBAL_PWR_MASK)
#define ETHSW_SPHY_CTRL_IDDQ_GLOBAL_PWR_MASK 0
#endif
static void sphy_ctrl_adjust(uint32_t ext_pwr_down)
{
    uint32_t phy_ctrl;

    if (ext_pwr_down == 0)
        return;

    phy_ctrl = *SPHY_CNTRL;
    phy_ctrl |= ETHSW_SPHY_CTRL_CK25_DIS_MASK |
                ETHSW_SPHY_CTRL_IDDQ_GLOBAL_PWR_MASK |
                ETHSW_SPHY_CTRL_IDDQ_BIAS_MASK |
                ETHSW_SPHY_CTRL_EXT_PWR_DOWN_MASK;
    *SPHY_CNTRL = phy_ctrl;
    printk("Adjusted SF2 SGPHY: sphy_ctrl=0x%08x\n", phy_ctrl);
}

static unsigned short gphy_base_addr(void)
{
    static unsigned short base_addr;

    if (!base_addr)
        BpGetGphyBaseAddress(&base_addr);

    return base_addr;
}

static uint32_t enabled_phys;

static int _phy_dev_add(phy_dev_t *phy_dev)
{
    unsigned short ba = gphy_base_addr();

    if (ba == BP_NOT_DEFINED)
        return -1;

    enabled_phys |= 1 << (phy_dev->addr - ba);
    return 0;
}

static int _phy_drv_init(phy_drv_t *phy_drv)
{
#if defined(QPHY_CNTRL)
    qphy_ctrl_adjust(~enabled_phys & 0x0f);
    sphy_ctrl_adjust(~enabled_phys & 0x10);
#else
    sphy_ctrl_adjust(~enabled_phys & 0x01);
#endif

    phy_drv->initialized = 1;
    return 0;
}

phy_drv_t phy_drv_sf2_gphy =
{
    .phy_type = PHY_TYPE_SF2_GPHY,
    .name = "GPHY",

    .power_get = mii_power_get,
    .power_set = mii_power_set,
    .apd_get = brcm_egphy_apd_get,
    .apd_set = brcm_egphy_apd_set,
    .eee_set = brcm_egphy_eee_set,
    .eee_get = brcm_egphy_eee_get,
    .eee_resolution_get = brcm_egphy_eee_resolution_get,
    .read_status = _phy_read_status,
    .speed_set = mii_speed_set,
    .caps_get = mii_caps_get,
    .caps_set = mii_caps_set,
    .phyid_get = mii_phyid_get,
    .isolate_phy = mii_isolate_phy,

    .read = brcm_egphy_read,
    .write = brcm_egphy_write,

    .init = phy_init,

    .dev_add = _phy_dev_add,
    .drv_init = _phy_drv_init,
    .loopback_set = brcm_loopback_set,
    .loopback_get = brcm_loopback_get,
    .apd_get = brcm_egphy_apd_get,
    .apd_set = brcm_egphy_apd_set,
    .cable_diag_run = brcm_cable_diag_run,
};


static int mac2mac_phy_init(phy_dev_t *phy_dev)
{
    // setup default speed, duplex and link state
    phy_dev->link = 0;
    phy_dev->speed = PHY_SPEED_1000;
    phy_dev->duplex = PHY_DUPLEX_FULL;
    return 0;
}

phy_drv_t phy_drv_mac2mac =
{
    .phy_type = PHY_TYPE_MAC2MAC,
    .name = "MAC2MAC",
    .init = mac2mac_phy_init,
};

#ifdef PHY_SF2_SERDES
#include "phy_drv_sf2_serdes.c"
#endif
