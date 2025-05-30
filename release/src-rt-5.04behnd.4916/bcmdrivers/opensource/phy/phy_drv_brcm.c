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
 *  Created on: Jan 2016
 *      Author: yuval.raviv@broadcom.com
 */

/*
 * Common functions for Broadcom phy drivers
 */

#include "bus_drv.h"
#include "phy_drv.h"
#include "phy_drv_mii.h"
#include "phy_drv_brcm.h"
#include "brcm_rgmii.h"

#define AUXSTAT                     0x19
#define BRCM_MIIEXT_BANK            0x1f

#define EXP_REG_BASE                0x0f00
#define EXP_REG_ADDRESS             0x17
#define EXP_REG_VALUE               0x15
#define RDB_REG_ADDRESS             0x1e
#define RDB_REG_VALUE               0x1f

#define CORE_SHD18_111              0x002f /* Miscellanous Control Register */
#define CORE_SHD1C_0A               0x001a /* Auto-Power Down */
#define CORE_SHD1C_05               0x0015 /* Reserved Control 3 */

#define EEE_CONTROL                 0x803d /* EEE Control */
#define EEE_RES_STAT                0x803e /* EEE Resolution Status */
#define EEE_ADV                     0x003c /* EEE Advertisement */
#define EEE_LP_ADV                  0x003d /* EEE Link Partner Advertisement */

#define REG_18                      0x18
#define REG_1C                      0x1c

#define REG_18_SHIFT                12
#define REG_1C_SHIFT                10

#define REG_18_AUX_CTRL             0x00
#define REG_18_MISC_CTRL            0x07

#define R18_AUX_EXT_PKT_LEN_EN      (1 << 14)
#define R18_MISC_FORCE_AUTO_MDIX    (1 <<  9)
#define R18_MISC_WIRESPEED_EN       (1 <<  4)

#define REG_1C_APD                  0x0a
#define REG_1C_SPARE_CTRL           0x05
#define REG_1C_CLK_ALIGN_CTRL       0x03

#define WRITE_ENABLE                0x8000

#define BRCM_PHY_REG_FORCE_LINK     (0x1<<12)
#define BRCM_PHY_TEST_REG           0x1e

int brcm_read_status(phy_dev_t *phy_dev)
{
    int ret = 0, mode;
    uint16_t val;

    if ((ret = phy_bus_read(phy_dev, AUXSTAT, &val)))
        goto exit_dn;

    phy_dev->link = ((val >> 2) & 0x1);

    if (!phy_dev->link)
        goto exit_dn;

    mode = ((val >> 8) & 0x7);

    if (mode == 1)
    {
        phy_dev->speed = PHY_SPEED_10;
        phy_dev->duplex = PHY_DUPLEX_HALF;
    }
    else if (mode == 2)
    {
        phy_dev->speed = PHY_SPEED_10;
        phy_dev->duplex = PHY_DUPLEX_FULL;
    }
    else if (mode == 3)
    {
        phy_dev->speed = PHY_SPEED_100;
        phy_dev->duplex = PHY_DUPLEX_HALF;
    }
    else if (mode == 5)
    {
        phy_dev->speed = PHY_SPEED_100;
        phy_dev->duplex = PHY_DUPLEX_FULL;
    }
    else if (mode == 6)
    {
        phy_dev->speed = PHY_SPEED_1000;
        phy_dev->duplex = PHY_DUPLEX_HALF;
    }
    else if (mode == 7)
    {
        phy_dev->speed = PHY_SPEED_1000;
        phy_dev->duplex = PHY_DUPLEX_FULL;
    }
    else
    {
        phy_dev->link = 0;
        return 0;
    }

    phy_dev->pause_rx = ((val >> 1) & 0x1);
    phy_dev->pause_tx = ((val >> 0) & 0x1);

    return 0;
    
exit_dn:
    phy_dev->link = 0;
    phy_dev->speed = PHY_SPEED_UNKNOWN;
    phy_dev->duplex = PHY_DUPLEX_UNKNOWN;
    phy_dev->pause_rx = 0;
    phy_dev->pause_tx = 0;
    return ret;
}

int brcm_read_status_rgmii_ib_override(phy_dev_t *phy_dev)
{
    int ret;

    if ((ret = brcm_read_status(phy_dev)))
        goto Exit;

    if (phy_dev->mii_type == PHY_MII_TYPE_RGMII)
        rgmii_ib_status_override(phy_dev->core_index, phy_dev->speed, phy_dev->duplex);

Exit:
    return ret;
}

int brcm_loopback_set(phy_dev_t *phy_dev, int enable, phy_speed_t speed)
{
    int rc; 
    uint32_t caps;
    uint16_t v16;
    int cur_loopback;
    phy_speed_t cur_speed;

    brcm_loopback_get(phy_dev, &cur_loopback, &cur_speed);

    if ((rc = phy_bus_read(phy_dev, MII_BMCR, &v16)))
        return rc;

    if(enable) {
        if (cur_loopback) {
            printk("Disable loop back first before enabling it.\n");
            return -1;
        }

        if (phy_dev->link) {
            printk("Disconnect the port before renabling loopback.\n");
            return -1;
        }

        if (speed == PHY_SPEED_UNKNOWN)
            speed = cascade_phy_max_speed_get(phy_dev);

        cascade_phy_dev_caps_get(phy_dev, CAPS_TYPE_ADVERTISE, &caps);
        if (!phy_dev_cap_speed_match(caps, speed)) {
            printk("Error: Speed %d Mbps is not supported by this PHY.\n", phy_speed_2_mbps(speed));
            return -1;
        }

        rc += phy_bus_read(phy_dev, MII_BMCR, &v16);
        phy_dev->loopback_save = v16;
        v16 &= ~(BMCR_SPEED1000|BMCR_SPEED100|BMCR_ANENABLE);
        v16 |= (speed==PHY_SPEED_1000?BMCR_SPEED1000:BMCR_SPEED100)|BMCR_LOOPBACK|BMCR_FULLDPLX;
        rc += phy_bus_write(phy_dev, MII_BMCR, v16);

        rc += phy_bus_read(phy_dev, BRCM_PHY_TEST_REG, &v16);
        v16 |= BRCM_PHY_REG_FORCE_LINK;
        rc += phy_bus_write(phy_dev, BRCM_PHY_TEST_REG, v16);
    }
    else {
        /* Only do disable if enable is done by the same API, not direct register operation */
        if (!phy_dev->loopback_save) {
            printk("Error: Loopback is not enabled\n");
            return -1;
        }

        rc += phy_bus_write(phy_dev, MII_BMCR, phy_dev->loopback_save);
        phy_dev->loopback_save = 0;

        rc += phy_bus_read(phy_dev, BRCM_PHY_TEST_REG, &v16);
        v16 &= ~BRCM_PHY_REG_FORCE_LINK;
        rc += phy_bus_write(phy_dev, BRCM_PHY_TEST_REG, v16);
    }
    return rc;
}

#define DSP_TAP13   0xd
int brcm_egphy_rx_short_ipg_en(phy_dev_t *phy_dev)
{
    uint16_t v16;

    if (PhyIsExtPhyId(phy_dev))
        phy_bus_write(phy_dev, REG_18, 0x0c00);  // enable clock for EXP_REG_ADDRESS to function

    // allow GPHY receive of shorten IPG due to brcm tag
    phy_bus_write(phy_dev, EXP_REG_ADDRESS, DSP_TAP13);
    phy_bus_read(phy_dev, EXP_REG_VALUE, &v16);

    phy_bus_write(phy_dev, EXP_REG_ADDRESS, DSP_TAP13);
    phy_bus_write(phy_dev, EXP_REG_VALUE, v16 | 0xc);  // set DSP_TAP13.b3-2 to 11= 8bytes

    return 0;
}

int brcm_loopback_get(phy_dev_t *phy_dev, int *enable, phy_speed_t *speed)
{
    int rc;
    uint16_t v16;

    if ((rc = phy_bus_read(phy_dev, MII_BMCR, &v16)))
        return rc;

    *enable = 0;
    if (phy_dev->loopback_save == 0)
        return 0;

    *enable = 1;
    rc += phy_bus_read(phy_dev, MII_BMCR, &v16);
    *speed = (v16&BMCR_SPEED1000)? PHY_SPEED_1000:PHY_SPEED_100;
 
    return rc;
}

static int brcm_shadow_rw(phy_dev_t *phy_dev, int write, int bank, uint16_t reg, uint16_t *val)
{
    int ret = 0;

    switch (bank)
    {
    case 1: 
        if ((ret = phy_bus_write(phy_dev, BRCM_MIIEXT_BANK, 0x8b)))
            goto Exit;
        break;
    case 2:
    case 3:
        if ((ret = phy_bus_write(phy_dev, BRCM_MIIEXT_BANK, 0x0f)))
            goto Exit;
        break;
    }

    if (bank == 3)
    {
        if (write)
        {
            if ((ret = phy_bus_write(phy_dev, 0x0e, reg)))
                goto Exit;

            if ((ret = phy_bus_write(phy_dev, 0x0f, *val)))
                goto Exit;
        }
        else
        {
            if ((ret = phy_bus_write(phy_dev, 0x0e, reg)))
                goto Exit;
            if ((ret = phy_bus_read(phy_dev, 0x0f, val)))
                goto Exit;
        }
    }
    else
    {
        if (write)
        {
            if ((ret = phy_bus_write(phy_dev, reg, *val)))
                goto Exit;
        }
        else 
        {
            if ((ret = phy_bus_read(phy_dev, reg, val)))
                goto Exit;
        }
    }

    if ((ret = phy_bus_write(phy_dev, BRCM_MIIEXT_BANK, 0x0b)))
            goto Exit;

Exit:
    return ret;
}

int brcm_shadow_read(phy_dev_t *phy_dev, int bank, uint16_t reg, uint16_t *val)
{
    return brcm_shadow_rw(phy_dev, 0, bank, reg, val);
}

int brcm_shadow_write(phy_dev_t *phy_dev, int bank, uint16_t reg, uint16_t val)
{
    return brcm_shadow_rw(phy_dev, 1, bank, reg, &val);
}

int brcm_exp_read(phy_dev_t *phy_dev, uint16_t reg, uint16_t *val)
{
    if (phy_bus_write(phy_dev, EXP_REG_ADDRESS, EXP_REG_BASE | reg))
        return -1;

    if (phy_bus_read(phy_dev, EXP_REG_VALUE, val))
        return -1;

    return 0;
}

int brcm_exp_write(phy_dev_t *phy_dev, uint16_t reg, uint16_t val)
{
    if (phy_bus_write(phy_dev, EXP_REG_ADDRESS, EXP_REG_BASE | reg))
        return -1;

    if (phy_bus_write(phy_dev, EXP_REG_VALUE, val))
        return -1;

    return 0;
}

int brcm_misc_read(phy_dev_t *phy_dev, uint16_t reg, int chnl, uint16_t *val)
{
    uint16_t tmp;
    if (brcm_shadow_18_read(phy_dev, 0, &tmp)) 
        return -1;
    tmp |= 0x800;
    if (phy_bus_write(phy_dev, REG_18, tmp)) 
        return -1;

    tmp = (chnl << 13)|reg;
    if (phy_bus_write(phy_dev, EXP_REG_ADDRESS, tmp)) 
        return -1;
    if (phy_bus_read(phy_dev, EXP_REG_VALUE, val)) 
        return -1;
    return 0;
}
EXPORT_SYMBOL(brcm_misc_read);

int brcm_misc_write(phy_dev_t *phy_dev, uint16_t reg, int chnl, uint16_t val)
{
    uint16_t tmp;
    if (brcm_shadow_18_read(phy_dev, 0, &tmp)) 
        return -1;
    tmp |= 0x800;
    if (phy_bus_write(phy_dev, REG_18, tmp)) 
        return -1;

    tmp = (chnl << 13)|reg;
    if (phy_bus_write(phy_dev, EXP_REG_ADDRESS, tmp)) 
        return -1;
    if (phy_bus_write(phy_dev, EXP_REG_VALUE, val)) 
        return -1;
    return 0;
}
EXPORT_SYMBOL(brcm_misc_write);

int brcm_rdb_set(phy_dev_t *phy_dev, int enable)
{
    if (enable)
        return brcm_exp_write(phy_dev, 0x7e, 0x0000);
    else
        return brcm_rdb_write(phy_dev, 0x87, 0x8000);
}

int brcm_rdb_read(phy_dev_t *phy_dev, uint16_t reg, uint16_t *val)
{
    if (phy_bus_write(phy_dev, RDB_REG_ADDRESS, reg))
        return -1;

    if (phy_bus_read(phy_dev, RDB_REG_VALUE, val))
        return -1;

    return 0;
}

int brcm_rdb_write(phy_dev_t *phy_dev, uint16_t reg, uint16_t val)
{
    if (phy_bus_write(phy_dev, RDB_REG_ADDRESS, reg))
        return -1;

    if (phy_bus_write(phy_dev, RDB_REG_VALUE, val))
        return -1;

    return 0;
}

int brcm_egphy_read(phy_dev_t *phy_dev, uint16_t reg, uint16_t *val)
{
    int ret;

    if (reg & RDB_ACCESS)
    {
        if ((ret = brcm_rdb_set(phy_dev, 1)))
            goto Exit;

        if ((ret = brcm_rdb_read(phy_dev, reg & ~RDB_ACCESS, val))) 
            goto Exit;

        if ((ret = brcm_rdb_set(phy_dev, 0)))
            goto Exit;
    }
    else
    {
        if ((ret = phy_bus_read(phy_dev, reg, val)))
            goto Exit;
    }

Exit:
    return ret;
}

int brcm_egphy_write(phy_dev_t *phy_dev, uint16_t reg, uint16_t val)
{
    int ret;

    if (reg & RDB_ACCESS)
    {
        if ((ret = brcm_rdb_set(phy_dev, 1)))
            goto Exit;

        if ((ret = brcm_rdb_write(phy_dev, reg & ~RDB_ACCESS, val))) 
            goto Exit;

        if ((ret = brcm_rdb_set(phy_dev, 0)))
            goto Exit;
    }
    else
    {
        if ((ret = phy_bus_write(phy_dev, reg, val)))
            goto Exit;
    }

Exit:
    return ret;
}

int brcm_egphy_force_auto_mdix_set(phy_dev_t *phy_dev, int enable)
{
    int ret;
    uint16_t val;

    if ((ret = phy_dev_read(phy_dev, RDB_ACCESS | CORE_SHD18_111, &val)))
        goto Exit;

    if (enable)
        val |= (1 << 9); /* Enable Force Auto MDIX */
    else
        val &= ~(1 << 9); /* Disable  Force Auto MDIX */

    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | CORE_SHD18_111, val)))
        goto Exit;

Exit:
    return ret;
}

int brcm_egphy_force_auto_mdix_get(phy_dev_t *phy_dev, int *enable)
{
    int ret;
    uint16_t val;

    if ((ret = phy_dev_read(phy_dev, RDB_ACCESS | CORE_SHD18_111, &val)))
        goto Exit;

    *enable = val & (1 << 9) ? 1 : 0; /* Force Auto MDIX Enabled */

Exit:
    return ret;
}

int brcm_egphy_eth_wirespeed_get(phy_dev_t *phy_dev, int *enable)
{
    int ret;
    uint16_t val;

    if ((ret = phy_dev_read(phy_dev, RDB_ACCESS | CORE_SHD18_111, &val)))
        goto Exit;

    *enable = val & (1 << 4) ? 1 : 0; /* Ethernet@Wirespeed Enabled */

Exit:
    return ret;
}


int brcm_egphy_eth_wirespeed_set(phy_dev_t *phy_dev, int enable)
{
    int ret;
    uint16_t val;

    if ((ret = phy_dev_read(phy_dev, RDB_ACCESS | CORE_SHD18_111, &val)))
        goto Exit;

    if (enable)
        val |= (1 << 4); /* Enable Ethernet@Wirespeed */
    else
        val &= ~(1 << 4); /* Disable Ethernet@Wirespeed */

    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | CORE_SHD18_111, val)))
        goto Exit;

Exit:
    return ret;
}

int brcm_egphy_apd_get(phy_dev_t *phy_dev, int *enable)
{
    int ret;
    uint16_t val;

    val = 0;

    if ((ret = phy_dev_read(phy_dev, RDB_ACCESS | CORE_SHD1C_0A, &val)))
        goto Exit;

    *enable = val & (1 << 5) ? 1 : 0; /* APD Enabled */

Exit:
    return ret;
}
int brcm_egphy_apd_set(phy_dev_t *phy_dev, int enable)
{
    int ret;
    uint16_t val;

    /* Auto-Power Down */
    val = 0;
    val |= (1 << 0); /* Counter for wakeup timer = 84 ms */

    if (enable)
    {
        val |= (1 << 5); /* Enable auto powerdown */
        val |= (1 << 8); /* Enable energy detect single link pulse */
    }

    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | CORE_SHD1C_0A, val)))
        goto Exit;

#if !defined(CONFIG_BCM963148)
    /* Reserved Control 3 */
    if ((ret = phy_dev_read(phy_dev, RDB_ACCESS | CORE_SHD1C_05, &val)))
        goto Exit;

    if (enable)
    {
        val |= (1 << 0); /* Disable CLK125 output */
        val &= ~(1 << 1); /* Enable powering down of dll during auto-power down */
    }
    else
    {
        val &= ~(1 << 0); /* Enable CLK125 output */
        val |= (1 << 1); /* Disable powering down of dll during auto-power down */
    }

    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | CORE_SHD1C_05, val)))
        goto Exit;
#endif

Exit:
    return ret;
}

int brcm_egphy_eee_get(phy_dev_t *phy_dev, int *enable)
{
    int ret;
    uint16_t val;

    /* local EEE Status */
    if ((ret = phy_bus_c45_read(phy_dev, 7, EEE_ADV, &val)))
        goto Exit;

    *enable = (val & (1 << 1) || val & (1 << 2));

Exit:
    return ret;
}

int brcm_egphy_eee_set(phy_dev_t *phy_dev, int enable)
{
    int ret;
    uint16_t val;

    /* EEE Control */
    val = 0;
    if (enable)
    {
        val |= (1 << 14); /* Enable EEE capability using SGMII auto-negotiation */
        val |= (1 << 15); /* Enable EEE LPI feature */
    }

    if ((ret = phy_bus_c45_write(phy_dev, 7, EEE_CONTROL, val)))
        goto Exit;

    /* EEE Advertisement */
    val = 0;
    if (enable)
    {
        val |= (1 << 1); /* EEE support for 100BASE-TX */
#if !defined(CONFIG_BCM963148)
        val |= (1 << 2); /* EEE support for 1000BASE-T */
#endif
    }

    if ((ret = phy_bus_c45_write(phy_dev, 7, EEE_ADV, val)))
        goto Exit;

    if ((ret = mii_autoneg_restart(phy_dev)))
        goto Exit;

Exit:
    return ret;
}

int brcm_egphy_eee_resolution_get(phy_dev_t *phy_dev, int *enable)
{
    int ret;
    uint16_t val;

    /* EEE Resolution Status */
    if ((ret = phy_bus_c45_read(phy_dev, 7, EEE_RES_STAT, &val)))
        goto Exit;

    /* Check if the link partner auto-negotiated EEE capability for 100BASE-TX or 1000BASE-T */
    *enable = (val & (1 << 1) || val & (1 << 2));

Exit:
    return ret;
}

int brcm_ephy_apd_get(phy_dev_t *phy_dev, int *enable)
{
    int ret;
    uint16_t val;

    /* Auxiliary Status 2 Register */
    if ((ret = brcm_shadow_read(phy_dev, 1, 0x1b, &val)))
        goto Exit;

    *enable = val & (1 << 5) ? 1 : 0; /* APD Enabled */

Exit:
    return ret;
}

int brcm_ephy_apd_set(phy_dev_t *phy_dev, int enable)
{
    uint16_t val;

    /* Auxiliary Status 2 Register */
    val = 0;
    val |= (1 << 0); /* Wake-up timer = 40 milliseconds */
    val |= (7 << 12); /* Cable Length > 140 meters */

    if (enable)
        val |= (1 << 5); /* APD Enable */

    return brcm_shadow_write(phy_dev, 1, 0x1b, val);
}

int brcm_ephy_eee_get(phy_dev_t *phy_dev, int *enable)
{
    int ret;
    uint16_t val;

    /* Auto-Negotiation EEE Advertisement */
    if ((ret = brcm_shadow_read(phy_dev, 3, 0x03, &val)))
        goto Exit;

    *enable = val & (1 << 1);

Exit:
    return ret;
}

int brcm_ephy_eee_set(phy_dev_t *phy_dev, int enable)
{
    int ret;
    uint16_t val;

    if (enable)
    {
        /* Auto-Negotiation EEE Advertisement */
        val = 0;
        val |= (1 << 1); /* EEE Auto-negotiation advertise */
        if ((ret = brcm_shadow_write(phy_dev, 3, 0x03, val)))
            goto Exit;

        /* PCS Control */
        val = 0;
        val |= (2 << 9); /* TACF Phase Track Mode = 2 */
        val |= (1 << 14); /* TACF Beta Control = 0.5 */
        if ((ret = brcm_shadow_write(phy_dev, 3, 0x06, val)))
            goto Exit;

        /* EEE Phase Search MSE Threshold */
        val = 0;
        val |= (0x50 << 0); /* MSE error threshold = 0x50 */
        if ((ret = brcm_shadow_write(phy_dev, 3, 0x0e, val)))
            goto Exit;

        /* Auto-Negotiation Status Register */
        val = 0;
        val |= (1 << 0); /* Null message enabled for auto-negotiation */
        val |= (1 << 1); /* EEE auto-negotiation enabled */
        if ((ret = brcm_shadow_write(phy_dev, 3, 0x0b, val)))
            goto Exit;
    }
    else
    {
        /* Auto-Negotiation EEE Advertisement */
        val = 0;
        if ((ret = brcm_shadow_write(phy_dev, 3, 0x03, val)))
            goto Exit;

        /* Auto-Negotiation Status Register */
        val = 0;
        if ((ret = brcm_shadow_write(phy_dev, 3, 0x0b, val)))
            goto Exit;
    }

    if ((ret = mii_autoneg_restart(phy_dev)))
        goto Exit;

Exit:
    return ret;
}

int brcm_ephy_eee_resolution_get(phy_dev_t *phy_dev, int *enable)
{
    int ret;
    uint16_t val;

    /* Auto-Negotiation Status Register */
    if ((ret = brcm_shadow_read(phy_dev, 3, 0x0b, &val)))
        goto Exit;

    /* Auto-negotiation EEE Enable, Auto-negotiation EEE Resolution */
    *enable = (val & (1 << 1) && val & (1 << 8));

Exit:
    return ret;
}

int brcm_shadow_18_read(phy_dev_t *phy_dev, uint16_t reg, uint16_t *val)
{
    int ret;

    *val = REG_18_MISC_CTRL | reg << REG_18_SHIFT;
    
    if ((ret = phy_bus_write(phy_dev, REG_18, *val)))
        goto Exit;

    if ((ret = phy_bus_read(phy_dev, REG_18, val)))
        goto Exit;

Exit:
    return ret;
}

int brcm_shadow_18_write(phy_dev_t *phy_dev, uint16_t reg, uint16_t val)
{
    int ret;

    val |= ((reg == REG_18_MISC_CTRL) ? WRITE_ENABLE : 0) | reg;
    
    if ((ret = phy_bus_write(phy_dev, REG_18, val)))
        goto Exit;

Exit:
    return ret;
}

int brcm_shadow_1c_read(phy_dev_t *phy_dev, uint16_t reg, uint16_t *val)
{
    int ret;

    *val = reg << REG_1C_SHIFT;
    
    if ((ret = phy_bus_write(phy_dev, REG_1C, *val)))
        goto Exit;

    if ((ret = phy_bus_read(phy_dev, REG_1C, val)))
        goto Exit;

Exit:
    return ret;
}

int brcm_shadow_1c_write(phy_dev_t *phy_dev, uint16_t reg, uint16_t val)
{
    val |= WRITE_ENABLE | reg << REG_1C_SHIFT;
    
    return phy_bus_write(phy_dev, REG_1C, val);
}


int brcm_shadow_18_ext_pkt_len_set(phy_dev_t *phy_dev, int enable)
{
    uint16_t val;
    int ret;

    if ((ret = brcm_shadow_18_read(phy_dev, REG_18_AUX_CTRL, &val)))
        goto Exit;

    if (enable)
        val |= R18_AUX_EXT_PKT_LEN_EN; /* Enable EXT_PKT_LEN */
    else
        val &= ~R18_AUX_EXT_PKT_LEN_EN; /* Disable  EXT_PKT_LEN */

    if ((ret = brcm_shadow_18_write(phy_dev, REG_18_AUX_CTRL, val)))
        goto Exit;

Exit:
    return ret;
}

int brcm_shadow_18_ext_pkt_len_get(phy_dev_t *phy_dev, int *enable)
{
    uint16_t val;
    int ret;

    if ((ret = brcm_shadow_18_read(phy_dev, REG_18_AUX_CTRL, &val)))
        goto Exit;

    *enable = (val & R18_AUX_EXT_PKT_LEN_EN) ? 1 : 0; /* EXT_PKT_LEN Enabled */

Exit:
    return ret;
}

int brcm_shadow_18_force_auto_mdix_set(phy_dev_t *phy_dev, int enable)
{
    uint16_t val;
    int ret;

    if ((ret = brcm_shadow_18_read(phy_dev, REG_18_MISC_CTRL, &val)))
        goto Exit;

    if (enable)
        val |= R18_MISC_FORCE_AUTO_MDIX; /* Enable Force Auto MDIX */
    else
        val &= ~R18_MISC_FORCE_AUTO_MDIX; /* Disable  Force Auto MDIX */

    if ((ret = brcm_shadow_18_write(phy_dev, REG_18_MISC_CTRL, val)))
        goto Exit;

Exit:
    return ret;
}

int brcm_shadow_18_force_auto_mdix_get(phy_dev_t *phy_dev, int *enable)
{
    uint16_t val;
    int ret;

    if ((ret = brcm_shadow_18_read(phy_dev, REG_18_MISC_CTRL, &val)))
        goto Exit;

    *enable = (val & R18_MISC_FORCE_AUTO_MDIX) ? 1 : 0; /* Force Auto MDIX Enabled */

Exit:
    return ret;
}

int brcm_shadow_18_eth_wirespeed_get(phy_dev_t *phy_dev, int *enable)
{
    uint16_t val;
    int ret;

    if ((ret = brcm_shadow_18_read(phy_dev, REG_18_MISC_CTRL, &val)))
        goto Exit;

    *enable = (val & R18_MISC_WIRESPEED_EN) ? 1 : 0; /* Ethernet@Wirespeed Enabled */

Exit:
    return ret;
}

int brcm_shadow_18_eth_wirespeed_set(phy_dev_t *phy_dev, int enable)
{
    uint16_t val;
    int ret;

    if ((ret = brcm_shadow_18_read(phy_dev, REG_18_MISC_CTRL, &val)))
        goto Exit;

    if (enable)
        val |= R18_MISC_WIRESPEED_EN; /* Enable Ethernet@Wirespeed */
    else
        val &= ~R18_MISC_WIRESPEED_EN; /* Disable Ethernet@Wirespeed */

    if ((ret = brcm_shadow_18_write(phy_dev, REG_18_MISC_CTRL, val)))
        goto Exit;

Exit:
    return ret;
}

int brcm_shadow_1c_apd_get(phy_dev_t *phy_dev, int *enable)
{
    uint16_t val;
    int ret;

    if ((ret = brcm_shadow_1c_read(phy_dev, REG_1C_APD, &val)))
        goto Exit;

    *enable = val & (1 << 5) ? 1 : 0; /* APD Enabled */

Exit:
    return ret;
}

int brcm_shadow_1c_apd_set(phy_dev_t *phy_dev, int enable)
{
    uint16_t val;
    int ret;

    val = (1 << 0); /* APD Wakeup timer */
    if (enable)
    {
        val |= (1 << 5); /* APD Enable */
        val |= (1 << 8); /* APD Compatibility */
    }

    if ((ret = brcm_shadow_1c_write(phy_dev, REG_1C_APD, val)))
        goto Exit;

    if ((ret = brcm_shadow_1c_read(phy_dev, REG_1C_SPARE_CTRL, &val)))
        goto Exit;

    if (enable)
    {
        val &= ~(1 << 0); /* Disable CLK125 Output */
        val &= ~(1 << 1); /* Enable CLK125 APD */
    }
    else
    {
        val |= (1 << 0); /* Enable CLK125 Output */
        val |= (1 << 1); /* Disable CLK125 APD */
    }

    if ((ret = brcm_shadow_1c_write(phy_dev, REG_1C_SPARE_CTRL, val)))
        goto Exit;

Exit:
    return ret;
}

int brcm_shadow_rgmii_init(phy_dev_t *phy_dev)
{
    uint16_t val;
    int ret;

    if ((ret = brcm_shadow_18_read(phy_dev, REG_18_MISC_CTRL, &val)))
        goto Exit;

    val |= (1 << 5); /* Disable RGMII Out of Band Status */;
    val |= (1 << 7); /* Enable RGMII Mode */

    if (phy_dev->delay_rx)
        val |= (1 << 8); /* Enable RGMII RXC Delay */
    else
        val &= ~(1 << 8); /* Disable RGMII RXC Delay */

    if ((ret = brcm_shadow_18_write(phy_dev, REG_18_MISC_CTRL, val)))
        goto Exit;

    if ((ret = brcm_shadow_1c_read(phy_dev, REG_1C_CLK_ALIGN_CTRL, &val)))
        goto Exit;

    if (phy_dev->delay_tx)
        val |= (1 << 9); /* Enable GTXCLK Delay */
    else
        val &= ~(1 << 9); /* Disable GTXCLK Delay */

    if ((ret = brcm_shadow_1c_write(phy_dev, REG_1C_CLK_ALIGN_CTRL, val)))
        goto Exit;

Exit:
    return ret;
}


static void phy_cable_diag_init(phy_dev_t *phy_dev)
{
#define TH_1_0	0x0102
#define TH_3_2	0x0208
#define TH_5_4	0x380F
#define PPW_1_0	0x5700
/* #define PPW_5_4_3_2	0x738E '4 probe setting */
#define PPW_5_4_3_2	0x92AC
/* #define PPWI_5_4_3_2_1_0	0x1000 */
#define PPWI_5_4_3_2_1_0	0x5000
#define PROBE_TYPE_6_5_4_3_2_1_0	0x9000

#define OFFSET_1_0	0x0E00
#define OFFSET_3_2	0x390B
/* #define OFFSET_3_2	0x3F18 */
#define OFFSET_5_4	0x0078

#define RX_CONFIG_1	0x0540
#define RX_CONFIG_2	0x0440
/* #define RX_CONFIG_3	0x0841 */
#define RX_CONFIG_3	0x0F40
#define RX_CONFIG_4	0x1345
#define RX_CONFIG_5	0x1523

#define START_0	0x0C00
#define START_3_2_1	0xC590
/* #define START_3_2_1	0x86C0 */
/* #define START_4_3	0x4B33 'old setting */
/* #define START_4_3	0x622E */
#define START_4_3	0x5C2E
#define START_5_4	0x2278
    uint16_t v16;

    if (phy_dev->flag & PHY_FLAG_CABLE_DIAG_INITED)
        return;

    brcm_exp_write(phy_dev, 0xC0, 0x0000);    /* disable Autoneg ECD */
    brcm_exp_write(phy_dev, 0xC7, 0xA01A);    /* EXPC7 frct_i_2 = 240, frct_i_1 = 4 */
    brcm_exp_write(phy_dev, 0xC8, 0x0300);    /* EXPC8 th_silent = 1 for 54382 ADC */
    brcm_exp_write(phy_dev, 0xC9, 0x00EF);    /* EXPC9 silent_c_th = 13, block_width_i = 9 */
    brcm_exp_write(phy_dev, 0xCB, 0x1304);    /* EXPCB  */
    brcm_exp_write(phy_dev, 0xCC, 0x0000);    /* EXPCC temp using 5 probe */
    brcm_exp_write(phy_dev, 0xCE, 0x4000);    /* EXPCE lp_drop_wait = 1, lp_safe_time = 5 */
    brcm_exp_write(phy_dev, 0xCF, 0x3000);    /* EXPCF disable both types of random starts */

    /* --------------------------------------------------------------------------------- */
    /* ------- page '01' */
    /* --------------------------------------------------------------------------------- */
    brcm_exp_write(phy_dev, 0xE0, TH_1_0);    /* EXPE0 set th[1] and th[0] value */
    brcm_exp_write(phy_dev, 0xE1, TH_3_2);    /* EXPE1 set th[3] & th[2] value */
    brcm_exp_write(phy_dev, 0xE2, TH_5_4);    /* EXPE2 restore exp. E2 */

    brcm_exp_write(phy_dev, 0xE3, PPW_1_0);    /* EXPE3 increase ppw[1] to 3, keep ppw[0] To 3 */
    brcm_exp_write(phy_dev, 0xE4, PPW_5_4_3_2);    /* EXPE4 default ppw[3] to 6 and default ppw[2] to 4 */
    brcm_exp_write(phy_dev, 0xE5, PPWI_5_4_3_2_1_0);    /* EXPE5  */
    brcm_exp_write(phy_dev, 0xE6, PROBE_TYPE_6_5_4_3_2_1_0);    /* EXPE6  */
    brcm_exp_write(phy_dev, 0xE7, 0xAA00);    /* EXPE7  */
    /* --------------------------------------------------------------------------------- */
    /* --- Now load values */
    brcm_exp_write(phy_dev, 0xEF, 0x40FF);    /* EXPEF write to shadow page 01' word 'E7', 'E6', 'E5', 'E4', 'E3', 'E2', 'E1', & ' E0' */
    brcm_exp_write(phy_dev, 0xCD, 0x1000);    /* EXPCD write strobe 1 */
    brcm_exp_write(phy_dev, 0xCD, 0x0000);    /* EXPCD write strobe 0 */
    brcm_exp_write(phy_dev, 0xE0, 0x0000);    /* EXPE0 restore exp. E0 */
    brcm_exp_write(phy_dev, 0xE1, 0x0000);    /* EXPE1 restore exp. E1 */
    brcm_exp_write(phy_dev, 0xE2, 0x0000);    /* EXPE2 restore exp. E2 */
    brcm_exp_write(phy_dev, 0xE3, 0x0000);    /* EXPE3 restore exp. E3 */
    brcm_exp_write(phy_dev, 0xE4, 0x0000);    /* EXPE4 restore exp. E4 */
    brcm_exp_write(phy_dev, 0xE5, 0x0000);    /* EXPE5 restore exp. E5 */
    brcm_exp_write(phy_dev, 0xE6, 0x0000);    /* EXPE6 restore exp. E6 */
    brcm_exp_write(phy_dev, 0xE7, 0x0000);    /* EXPE7 restore exp. E7 */
    brcm_exp_write(phy_dev, 0xEF, 0x0000);    /* EXPEF restore exp. EF */

    /* --------------------------------------------------------------------------------- */
    /* ------- page '10' */
    /* --------------------------------------------------------------------------------- */

    brcm_exp_write(phy_dev, 0xE0, OFFSET_1_0);    /* EXPE0  */
    brcm_exp_write(phy_dev, 0xE1, OFFSET_3_2);    /* EXPE1  */
    brcm_exp_write(phy_dev, 0xE2, OFFSET_5_4);    /* EXPE2  */
    /* --------------------------------------------------------------------------------- */
    /* --- Now load values */
    brcm_exp_write(phy_dev, 0xEF, 0x8007);    /* EXPEF write to shadow page */
    brcm_exp_write(phy_dev, 0xCD, 0x1000);    /* EXPCD write strobe 1 */
    brcm_exp_write(phy_dev, 0xCD, 0x0000);    /* EXPCD write strobe 0 */

    brcm_exp_write(phy_dev, 0xE0, 0x0000);    /* EXPE0 restore exp. E0 */
    brcm_exp_write(phy_dev, 0xE1, 0x0000);    /* EXPE1 restore exp. E1 */
    brcm_exp_write(phy_dev, 0xE2, 0x0000);    /* EXPE2 restore exp. E2 */
    brcm_exp_write(phy_dev, 0xEF, 0x0000);    /* EXPEF restore exp. EF */
    /* --------------------------------------------------------------------------------- */
    /* ------- page '00' */
    /* --------------------------------------------------------------------------------- */
    brcm_exp_write(phy_dev, 0xE0, 0x0001);    /* EXPE0  */
    brcm_exp_write(phy_dev, 0xE1, RX_CONFIG_1);    /* EXPE1  */
    brcm_exp_write(phy_dev, 0xE2, RX_CONFIG_2);    /* EXPE2  */
    brcm_exp_write(phy_dev, 0xE3, RX_CONFIG_3);    /* EXPE3  */
    brcm_exp_write(phy_dev, 0xE4, RX_CONFIG_4);    /* EXPE4  */
    brcm_exp_write(phy_dev, 0xE5, RX_CONFIG_5);    /* EXPE5  */
    /* App.WrExp 0, 0x00E7, START_0 */
    brcm_exp_write(phy_dev, 0xE8, START_3_2_1);    /* EXPE8  */
    brcm_exp_write(phy_dev, 0xE9, START_4_3);    /* EXPE9  */
    brcm_exp_write(phy_dev, 0xEA, START_5_4);    /* EXPEA  */
    /* --------------------------------------------------------------------------------- */
    brcm_exp_write(phy_dev, 0xCD, 0x00D0);    /* EXPCD  */
    /* --------------------------------------------------------------------------------- */
    /* --Now we have finished ECD parameter loading. */
    /* flush out old results */
    brcm_exp_read(phy_dev, 0xC0, &v16);    /* dummy read to flush out sticky bit in exp.C0 */
    brcm_exp_write(phy_dev, 0xC0, 0x0000);    /* EXPC0 disable Autoneg ECD */

    phy_dev->flag |= PHY_FLAG_CABLE_DIAG_INITED;
}

/* 
   Work around some hardware inconsistency
   Pick up the most popular length from 4 pairs
 */
static void cable_length_pick_link_up(int *pair_len, int excluded_pair)
{
    int len[4]={0};
    int i, j, k, m;

    for (i=0, k=0; i<4; i++) {
        if (excluded_pair & (1<<i))  /* Exclude failed CD pair */
            continue;
        
        for(j=0; j<k; j++) 
            if (pair_len[j] == pair_len[i]) 
                break;
        if (j==k)
            k++;
        len[j]++;
    }

    for (i=0, j=0, m=0; i<k; i++) {
        if (len[i] == 0)    /* If result is zero, exclude the pair from picking */
            continue;
        
        if(len[i]>j) {
            j=len[i];
            m=i;
        }
    }

    m = pair_len[m];
    for (i=0; i<4; i++)
        pair_len[i] = m; 
}

int brcm_cable_diag_run(phy_dev_t *phy_dev)
{
    uint16_t v16, ctrl_status;
    int i, j, result = 0, ret = 0, excluded_pair = 0;
    int apd_enabled, phy_link;
    int pair_len[4];
    unsigned long jiffie;
    int retries = 0;
    ethcd_t *ethcd = &phy_dev->ethcd;
#define ECD_CHECK_SECS 3
#define ECD_MAX_RETRIES 3

    phy_dev_apd_get(phy_dev, &apd_enabled);
    if (apd_enabled)
        phy_dev_apd_set(phy_dev, 0);

    phy_cable_diag_init(phy_dev);

TryAgain:
    if (retries) for(i=0, jiffie = jiffies; jiffies < (jiffie + msecs_to_jiffies(2*1000)););
    if (++retries > ECD_MAX_RETRIES)  /* If we did retry more than certain time, declares it as faiure */
        goto end;

    v16 = ECD_RUN_IMMEDIATE;
    if ((phy_link = phy_dev->link))
        v16 |= ECD_BREAK_LINK; 
    brcm_exp_write(phy_dev, MII_ECD_CTRL_STATUS, v16);

    if (phy_link) { /* If link is up, Write RUN first and wait until link goes down */
        for(;;) {
            phy_dev_read_status(phy_dev);
            if (!phy_dev->link) break;
        }
    }

    for(i=0, jiffie = jiffies; jiffies < (jiffie + msecs_to_jiffies(ECD_CHECK_SECS*1000)); ) {
        brcm_exp_read(phy_dev, MII_ECD_CTRL_STATUS, &v16);
        if (!(v16 & ECD_DIAG_IN_PROG)) {
            i = 1;
            break;
        }
    }
    ctrl_status = v16;

    if (!i) {
        result = PA_CD_CODE_INVALID;
        ret = -1;
        goto TryAgain;
    }

    for(i=0, jiffie = jiffies; jiffies < (jiffie + msecs_to_jiffies(ECD_CHECK_SECS*1000)); ) { /* Check if all four pairs of diags are done */
        ret = brcm_exp_read(phy_dev, MII_ECD_CTRL_FAULT_TYPE, &v16);
        result = v16;
        excluded_pair = 0;
        for(j=0; j<4; j++) { /* Check if all four pairs of diags are done */
            if( PA_CD_CODE_PAIR_GET(result, j) > PA_CD_CODE_PAIR_INTER_SHORT) 
                break;

            /* If link is up, excluded failed measuring result */
            if( phy_link && ( PA_CD_CODE_PAIR_GET(result, j) != PA_CD_CODE_PAIR_OK))
                excluded_pair |= (1<<j); 
        }

        /* If all pair of diags finish, check the results */
        if (j==4) {
            /* If in link up, all pair diag failed, try again */
            if (result == PA_CD_CODE_INVALID || excluded_pair == 0xf ) 
                goto TryAgain;
            /* Otherwise, we are done with CD */
            i=1;
            break;
        }
    }

    if (phy_link)
        result = PA_CD_CODE_PAIR_ALL_OK;

    if (result == PA_CD_CODE_INVALID || !i) {
        result = PA_CD_CODE_INVALID;
        ret = -1;
        goto TryAgain;
    }

#define CABLE_LEN_OFFSET_LINK_DOWN 100
    for(i=0; i<4; i++) {
        ret |= brcm_exp_read(phy_dev, MII_ECD_CABLE_LEN+i, &v16);
        if (result == PA_CD_CODE_PAIR_ALL_OPEN)
            pair_len[i] = (v16> CABLE_LEN_OFFSET_LINK_DOWN ? v16 - CABLE_LEN_OFFSET_LINK_DOWN : 0); /* To guarrantee no cable result correct based on testing */
        else
            pair_len[i] = v16;
    }

    /* If link is up, but alll pair length is zero, try again */
    if (phy_link && (pair_len[0] + pair_len[1] + pair_len[2] + pair_len[3] == 0))
        goto TryAgain;

end:
    /* If link was up, poll until link comes back due to CD */
    if (phy_link) {
        for(jiffie = jiffies; jiffies < (jiffie + msecs_to_jiffies(3*ECD_CHECK_SECS*1000)); ) {
            phy_dev_read_status(phy_dev);
            if (phy_dev->link) break;
        }
    }
            
    if (result == PA_CD_CODE_PAIR_ALL_OK || phy_dev->link)
        cable_length_pick_link_up(pair_len, excluded_pair);

    if (apd_enabled)
        phy_dev_apd_set(phy_dev, apd_enabled);

    memcpy(ethcd->pair_len_cm, pair_len, sizeof(pair_len));

    ethcd->return_value = ETHCD_OK;
    switch(result)
    {
        case PA_CD_CODE_INVALID:
        	ethcd->cable_code = ETHCD_STATUS_INVALID;
            ethcd->return_value = ETHCD_FAILED;
            break;
        case PA_CD_CODE_PAIR_ALL_OK:
            ethcd->cable_code = ETHCD_STATUS_GOOD_CONNECTED;
            break;
        case PA_CD_CODE_PAIR_ALL_OPEN:
            if ((pair_len[0] + pair_len[1] + pair_len[2] + pair_len[3]) == 0)
                ethcd->cable_code = ETHCD_STATUS_NO_CABLE;
            else if (pair_len[0] == pair_len[1] && pair_len[0] == pair_len[2] && pair_len[0] == pair_len[3])
                ethcd->cable_code = ETHCD_STATUS_GOOD_OPEN;
            else
                ethcd->cable_code = ETHCD_STATUS_BAD_OPEN;
            break;
        default:
            ethcd->cable_code = ETHCD_STATUS_MIXED_BAD;
            for (i=0; i<4; i++)
            {
                switch(PA_CD_CODE_PAIR_GET(result, i))
                {
                    case PA_CD_CODE_PAIR_OK:
                        ethcd->pair_code[i] = ETHCD_PAIR_OK;
                        break;
                    case PA_CD_CODE_PAIR_OPEN:
                        ethcd->pair_code[i] = ETHCD_PAIR_OPEN;
                        break;
                    case PA_CD_CODE_PAIR_INTRA_SHORT:
                        ethcd->pair_code[i] = ETHCD_PAIR_INTRA_SHORT;
                        break;
                    case PA_CD_CODE_PAIR_INTER_SHORT:
                        ethcd->pair_code[i] = ETHCD_PAIR_INTER_SHORT;
                        break;
                    default:
                        ethcd->pair_code[i] = ETHCD_PAIR_UNKNOWN;
                        break;
                }            
            }
            break;
    }

    ethcd->link = phy_link;
    ethcd->time_stamp = get_epoch_time64();
    ethcd->flag |= ETHCD_FLAG_DATA_VALID;

    return ret;
}

#ifdef _CFE_
#define mutex_lock(x)
#define mutex_unlock(x)
#else
#include <linux/spinlock.h>
DEFINE_MUTEX(bcm_phy_exp_mutex);
#endif

#define BRCM_MIIEXT_BANK            0x1f
#define BRCM_MIIEXT_BANK_MASK       0xfff0
#define BRCM_MIIEXT_ADDR_RANGE      0xffe0
#define BRCM_MIIEXT_DEF_BANK        0x8000
#define BRCM_MIIEXT_OFFSET          0x10
#define BRCM_MIIEXT_OFF_MASK        0x0f

int ethsw_phy_exp_rw(phy_dev_t *phy_dev, uint32_t reg, uint16_t *v16_p, int rd)
{
    u32 bank, offset;
    int rc = -1;
    phy_type_t phy_type = PHY_TYPE_UNKNOWN;

    if (phy_dev->phy_drv)
        phy_type = phy_dev->phy_drv->phy_type;

    if (!IsC45Phy(phy_dev)) {
        if (reg < 0x20) {   /* CL22 space */
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
    }
#if defined(PHY_EXT3)
    else if (reg < 0x200000) /* CL45 space */
    {
        if (reg < 0x20 && IsC45Phy(phy_dev)) /* Remapping CL22 register for Broadcom EXT3 PHY to 0x7.ffe0 */
            reg += 0x7ffe0;

        if (rd)
            rc = phy_bus_c45_read32(phy_dev, reg, v16_p);
        else
            rc = phy_bus_c45_write32(phy_dev, reg, *v16_p);
    }
#endif

    return rc;
}
EXPORT_SYMBOL(ethsw_phy_exp_rw);
