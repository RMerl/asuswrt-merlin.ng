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

#define REG_18_MISC_CTRL            0x07

#define REG_1C_APD                  0x0a
#define REG_1C_SPARE_CTRL           0x05
#define REG_1C_CLK_ALIGN_CTRL       0x03

#define WRITE_ENABLE                0x8000

int brcm_read_status(phy_dev_t *phy_dev)
{
    int ret, mode;
    uint16_t val;
    phy_dev->link = 0;
    phy_dev->speed = PHY_SPEED_UNKNOWN;
    phy_dev->duplex = PHY_DUPLEX_UNKNOWN;
    phy_dev->pause_rx = 0;
    phy_dev->pause_tx = 0;

    if ((ret = phy_bus_read(phy_dev, AUXSTAT, &val)))
        return ret;

    phy_dev->link = ((val >> 2) & 0x1);

    if (!phy_dev->link)
        return 0;

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
        val |= (1 << 2); /* EEE support for 1000BASE-T */
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

    *val = reg | reg << REG_18_SHIFT;
    
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

    val |= WRITE_ENABLE | reg | reg << REG_18_SHIFT;
    
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
    int ret;

    val |= WRITE_ENABLE | reg << REG_1C_SHIFT;
    
    if ((ret = phy_bus_write(phy_dev, REG_1C, val)))
        goto Exit;
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
        val |= (1 << 9); /* Enable Force Auto MDIX */
    else
        val &= ~(1 << 9); /* Disable  Force Auto MDIX */

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

    *enable = val & (1 << 9) ? 1 : 0; /* Force Auto MDIX Enabled */

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
        val |= (1 << 4); /* Enable Ethernet@Wirespeed */
    else
        val &= ~(1 << 4); /* Disable Ethernet@Wirespeed */

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

static int mii_exp_write(phy_dev_t *phy_dev, int reg, int val)
{
    int ret;
    ret = phy_bus_write(phy_dev, MII_EXPAND_REG_REG, reg|MII_EXPAND_REG_MARK);
    ret |= phy_bus_write(phy_dev, MII_EXPAND_REG_VAL, val);
    return ret;
}

static int mii_exp_read(phy_dev_t *phy_dev, int reg, uint16_t *val)
{
    int ret;
    ret = phy_bus_write(phy_dev, MII_EXPAND_REG_REG, reg|MII_EXPAND_REG_MARK);
    ret |= phy_bus_read(phy_dev, MII_EXPAND_REG_VAL, val);
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

    mii_exp_write(phy_dev, 0xC0, 0x0000);    /* disable Autoneg ECD */
    mii_exp_write(phy_dev, 0xC7, 0xA01A);    /* EXPC7 frct_i_2 = 240, frct_i_1 = 4 */
    mii_exp_write(phy_dev, 0xC8, 0x0300);    /* EXPC8 th_silent = 1 for 54382 ADC */
    mii_exp_write(phy_dev, 0xC9, 0x00EF);    /* EXPC9 silent_c_th = 13, block_width_i = 9 */
    mii_exp_write(phy_dev, 0xCB, 0x1304);    /* EXPCB  */
    mii_exp_write(phy_dev, 0xCC, 0x0000);    /* EXPCC temp using 5 probe */
    mii_exp_write(phy_dev, 0xCE, 0x4000);    /* EXPCE lp_drop_wait = 1, lp_safe_time = 5 */
    mii_exp_write(phy_dev, 0xCF, 0x3000);    /* EXPCF disable both types of random starts */

    /* --------------------------------------------------------------------------------- */
    /* ------- page '01' */
    /* --------------------------------------------------------------------------------- */
    mii_exp_write(phy_dev, 0xE0, TH_1_0);    /* EXPE0 set th[1] and th[0] value */
    mii_exp_write(phy_dev, 0xE1, TH_3_2);    /* EXPE1 set th[3] & th[2] value */
    mii_exp_write(phy_dev, 0xE2, TH_5_4);    /* EXPE2 restore exp. E2 */

    mii_exp_write(phy_dev, 0xE3, PPW_1_0);    /* EXPE3 increase ppw[1] to 3, keep ppw[0] To 3 */
    mii_exp_write(phy_dev, 0xE4, PPW_5_4_3_2);    /* EXPE4 default ppw[3] to 6 and default ppw[2] to 4 */
    mii_exp_write(phy_dev, 0xE5, PPWI_5_4_3_2_1_0);    /* EXPE5  */
    mii_exp_write(phy_dev, 0xE6, PROBE_TYPE_6_5_4_3_2_1_0);    /* EXPE6  */
    mii_exp_write(phy_dev, 0xE7, 0xAA00);    /* EXPE7  */
    /* --------------------------------------------------------------------------------- */
    /* --- Now load values */
    mii_exp_write(phy_dev, 0xEF, 0x40FF);    /* EXPEF write to shadow page 01' word 'E7', 'E6', 'E5', 'E4', 'E3', 'E2', 'E1', & ' E0' */
    mii_exp_write(phy_dev, 0xCD, 0x1000);    /* EXPCD write strobe 1 */
    mii_exp_write(phy_dev, 0xCD, 0x0000);    /* EXPCD write strobe 0 */
    mii_exp_write(phy_dev, 0xE0, 0x0000);    /* EXPE0 restore exp. E0 */
    mii_exp_write(phy_dev, 0xE1, 0x0000);    /* EXPE1 restore exp. E1 */
    mii_exp_write(phy_dev, 0xE2, 0x0000);    /* EXPE2 restore exp. E2 */
    mii_exp_write(phy_dev, 0xE3, 0x0000);    /* EXPE3 restore exp. E3 */
    mii_exp_write(phy_dev, 0xE4, 0x0000);    /* EXPE4 restore exp. E4 */
    mii_exp_write(phy_dev, 0xE5, 0x0000);    /* EXPE5 restore exp. E5 */
    mii_exp_write(phy_dev, 0xE6, 0x0000);    /* EXPE6 restore exp. E6 */
    mii_exp_write(phy_dev, 0xE7, 0x0000);    /* EXPE7 restore exp. E7 */
    mii_exp_write(phy_dev, 0xEF, 0x0000);    /* EXPEF restore exp. EF */

    /* --------------------------------------------------------------------------------- */
    /* ------- page '10' */
    /* --------------------------------------------------------------------------------- */

    mii_exp_write(phy_dev, 0xE0, OFFSET_1_0);    /* EXPE0  */
    mii_exp_write(phy_dev, 0xE1, OFFSET_3_2);    /* EXPE1  */
    mii_exp_write(phy_dev, 0xE2, OFFSET_5_4);    /* EXPE2  */
    /* --------------------------------------------------------------------------------- */
    /* --- Now load values */
    mii_exp_write(phy_dev, 0xEF, 0x8007);    /* EXPEF write to shadow page */
    mii_exp_write(phy_dev, 0xCD, 0x1000);    /* EXPCD write strobe 1 */
    mii_exp_write(phy_dev, 0xCD, 0x0000);    /* EXPCD write strobe 0 */

    mii_exp_write(phy_dev, 0xE0, 0x0000);    /* EXPE0 restore exp. E0 */
    mii_exp_write(phy_dev, 0xE1, 0x0000);    /* EXPE1 restore exp. E1 */
    mii_exp_write(phy_dev, 0xE2, 0x0000);    /* EXPE2 restore exp. E2 */
    mii_exp_write(phy_dev, 0xEF, 0x0000);    /* EXPEF restore exp. EF */
    /* --------------------------------------------------------------------------------- */
    /* ------- page '00' */
    /* --------------------------------------------------------------------------------- */
    mii_exp_write(phy_dev, 0xE0, 0x0001);    /* EXPE0  */
    mii_exp_write(phy_dev, 0xE1, RX_CONFIG_1);    /* EXPE1  */
    mii_exp_write(phy_dev, 0xE2, RX_CONFIG_2);    /* EXPE2  */
    mii_exp_write(phy_dev, 0xE3, RX_CONFIG_3);    /* EXPE3  */
    mii_exp_write(phy_dev, 0xE4, RX_CONFIG_4);    /* EXPE4  */
    mii_exp_write(phy_dev, 0xE5, RX_CONFIG_5);    /* EXPE5  */
    /* App.WrExp 0, 0x00E7, START_0 */
    mii_exp_write(phy_dev, 0xE8, START_3_2_1);    /* EXPE8  */
    mii_exp_write(phy_dev, 0xE9, START_4_3);    /* EXPE9  */
    mii_exp_write(phy_dev, 0xEA, START_5_4);    /* EXPEA  */
    /* --------------------------------------------------------------------------------- */
    mii_exp_write(phy_dev, 0xCD, 0x00D0);    /* EXPCD  */
    /* --------------------------------------------------------------------------------- */
    /* --Now we have finished ECD parameter loading. */
    /* flush out old results */
    mii_exp_read(phy_dev, 0xC0, &v16);    /* dummy read to flush out sticky bit in exp.C0 */
    mii_exp_write(phy_dev, 0xC0, 0x0000);    /* EXPC0 disable Autoneg ECD */

    phy_dev->flag |= PHY_FLAG_CABLE_DIAG_INITED;
}

/* 
   Work around some hardware inconsistency
   Pick up the most popular length from 4 pairs
 */
static void cable_length_pick_link_up(int *pair_len)
{
    int len[4]={0};
    int i, j, k, m;

    for (i=0, k=0; i<4; i++) {
        for(j=0; j<k; j++) 
            if (pair_len[j] == pair_len[i]) 
                break;
        if (j==k)
            k++;
        len[j]++;
    }

    for (i=0, j=0, m=0; i<k; i++)
        if(len[i]>j) {
            j=len[i];
            m=i;
        }

    m = pair_len[m];
    for (i=0; i<4; i++)
        pair_len[i] = m; 
}

#if defined(_CFE_)
static unsigned long _jiffies;
#define jiffies (_jiffies++)
#define msecs_to_jiffies(j) ((j)*10)
#endif

int brcm_cable_diag_run(phy_dev_t *phy_dev, int *result, int *pair_len)
{
    uint16_t v16;
    int i, j, ret = 0;
    int apd_enabled;
    unsigned long jiffie;
    int old_link;
#define ECD_CHECK_SECS 3

    phy_dev_apd_get(phy_dev, &apd_enabled);
    if (apd_enabled)
        phy_dev_apd_set(phy_dev, 0);

    phy_cable_diag_init(phy_dev);
    v16 = ECD_RUN_IMMEDIATE;
    if (phy_dev->link)
        v16 |= ECD_BREAK_LINK; 
    mii_exp_write(phy_dev, MII_ECD_CTRL_STATUS, v16);

    for(i=0, jiffie = jiffies; jiffies < (jiffie + msecs_to_jiffies(ECD_CHECK_SECS*1000)); ) {
        mii_exp_read(phy_dev, MII_ECD_CTRL_STATUS, &v16);
        if (!(v16 & ECD_DIAG_IN_PROG)) {
            i = 1;
            break;
        }
    }

    if (!i) {
        *result = PA_CD_CODE_INVALID;
        ret = -1;
        goto end;
    }

    for(i=0, jiffie = jiffies; jiffies < (jiffie + msecs_to_jiffies(ECD_CHECK_SECS*1000)); ) {
        ret = mii_exp_read(phy_dev, MII_ECD_CTRL_FAULT_TYPE, &v16);
        *result = v16;
        /* Check if we finished with no error */
        for(j=0; j<4; j++) {
            if( PA_CD_CODE_PAIR_GET(*result, j) > PA_CD_CODE_PAIR_INTER_SHORT ||
                PA_CD_CODE_PAIR_GET(*result, j) == PA_CD_CODE_INVALID) 
                break;
        }

        /* If invalid happens, continue next round check */
        if (j<4)
            continue;

        i=1;
        break;
    }

    if (!i) {
        *result = PA_CD_CODE_INVALID;
        ret = -1;
        goto end;
    }

    for(i=0; i<4; i++) {
        ret |= mii_exp_read(phy_dev, MII_ECD_CABLE_LEN+i, &v16);
        pair_len[i] = v16;
    }

end:
    /* If link was up, poll until link comes back due to CD */
    old_link = phy_dev->link;
    if (phy_dev->link) {
        for(jiffie = jiffies; jiffies < (jiffie + msecs_to_jiffies(3*ECD_CHECK_SECS*1000)); ) {
            phy_dev_read_status(phy_dev);
            if (phy_dev->link) break;
        }
    }

    if (old_link && !phy_dev->link)
        printk("Link down due to Cable Diag Operation.\n");
            
    if (phy_dev->link)
        cable_length_pick_link_up(pair_len);

    if (apd_enabled)
        phy_dev_apd_set(phy_dev, apd_enabled);

    return ret;
}


