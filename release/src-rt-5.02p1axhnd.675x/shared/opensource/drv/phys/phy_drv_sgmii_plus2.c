/*
   Copyright (c) 2016 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2016:DUAL/GPL:standard

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
 *  Created on: Aug 2016
 *      Author: yuval.raviv@broadcom.com
 */

/*
 * Common functions for SGMIIPLUS2 core
 */

#include "bus_drv.h"
#include "phy_drv.h"
#include "phy_drv_mii.h"
#include "phy_drv_sgmii_plus2.h"
#include "bcm_gpio.h"

#define XGXS_STATUS1     0x8122

static prog_entry_t toggle_pll75[] = {
    { "XGXSBlk2 Block Address",         0x1f, 0x8100 },
    { "Override pin, use mdio",         0x16, 0x0020 },
    { "TxPLL Block Address",            0x1f, 0x8050 },
    { "Toggle mmd_resetb to 0",         0x14, 0x8021 },
    { "Toggle mmd_resetb to 1",         0x14, 0x8821 },
    { "BLK0 Block Address",             0x1f, 0x8000 },
    { NULL,                             0x00, 0x0000 }
};

static prog_entry_t refclk50m_vco6p25g[] = {
    { "Digital Block Address",          0x1f, 0x8300 },
    { "refclk_sel=3'h6 (50 MHz)",       0x18, 0xc000 },
    { "PLL AFE Block Address",          0x1f, 0x8050 },
    { "pll_afectrl0",                   0x10, 0x5740 },
    { "pll_afectrl1",                   0x11, 0x01d0 },
    { "pll_afectrl2",                   0x12, 0x19f0 },
    { "pll_afectrl3",                   0x13, 0xaab0 },
    { "pll_afectrl4",                   0x14, 0x8821 },
    { "pll_afectrl5",                   0x15, 0x0044 },
    { "pll_afectrl6",                   0x16, 0x8000 },
    { "pll_afectrl7",                   0x17, 0x0872 },
    { "pll_afectrl8",                   0x18, 0x0000 },
    { "BLK0 Block Address",             0x1f, 0x8000 },
    { NULL,                             0x00, 0x0000 }
};

static prog_entry_t refclk80m_vco6p25g[] = {
    { "Digital Block Address",          0x1f, 0x8300 },
    { "refclk_sel=3'h7 (80 MHz)",       0x18, 0xe000 },
    { "PLL AFE Block Address",          0x1f, 0x8050 },
    { "pll_afectrl0",                   0x10, 0x5740 },
    { "pll_afectrl1",                   0x11, 0x01d0 },
    { "pll_afectrl2",                   0x12, 0x59f0 },
    { "pll_afectrl3",                   0x13, 0xaa80 },
    { "pll_afectrl4",                   0x14, 0x8821 },
    { "pll_afectrl5",                   0x15, 0x0044 },
    { "pll_afectrl6",                   0x16, 0x8800 },
    { "pll_afectrl7",                   0x17, 0x0813 },
    { "pll_afectrl8",                   0x18, 0x0000 },
    { "BLK0 Block Address",             0x1f, 0x8000 },
    { NULL,                             0x00, 0x0000 }
};

static prog_entry_t refclk125m_vco6p25g[] = {
    { "Digital Block Address",          0x1f, 0x8300 },
    { "refclk_sel=3'h2 (125 MHz)",      0x18, 0x4000 },
    { "PLL AFE Block Address",          0x1f, 0x8050 },
    { "pll_afectrl0",                   0x10, 0x5740 },
    { "pll_afectrl1",                   0x11, 0x01d0 },
    { "pll_afectrl2",                   0x12, 0x19f0 },
    { "pll_afectrl3",                   0x13, 0x2b00 },
    { "pll_afectrl4",                   0x14, 0x0023 },
    { "pll_afectrl5",                   0x15, 0x0044 },
    { "pll_afectrl6",                   0x16, 0x0000 },
    { "pll_afectrl7",                   0x17, 0x0000 },
    { "pll_afectrl8",                   0x18, 0x0000 },
    { "BLK0 Block Address",             0x1f, 0x8000 },
    { NULL,                             0x00, 0x0000 }
};

static prog_entry_t disable_afe_limit_amp[] = {
    { "RX3 Slicer",                     0x1f, 0x8480 },
    { "disable lmtcal",                 0x12, 0x83f8 },
    { "BLK0 Block Address",             0x1f, 0x8000 },
    { NULL,                             0x00, 0x0000 }
};

static prog_entry_t disable_pll[] = {
    { "BLK0 Block Address",             0x1f, 0x8000 },
    { "disable pll start sequencer",    0x10, 0x0c2f },
    { NULL,                             0x00, 0x0000 }
}; 

static prog_entry_t enable_pll[] = {
    { "BLK0 Block Address",             0x1f, 0x8000 },
    { "enable pll start sequencer",     0x10, 0x2c2f },
    { NULL,                             0x00, 0x0000 }
}; 

static prog_entry_t forced_speed_sgmii_os5[] = {
    { "Digital Block Address",          0x1f, 0x8300 },
    { "enable sgmii mode",              0x10, 0x0100 },
    { "RX2 Block Address",              0x1f, 0x8470 },
    { "Set step_one[1:0]",              0x13, 0x1251 },
    { "Digital5 Block Address",         0x1f, 0x8340 },
    { "set oversampling mode",          0x1a, 0x0003 },
    { "BLK0 Block Address",             0x1f, 0x8000 },
    { NULL,                             0x00, 0x0000 }
}; 

static prog_entry_t forced_speed_2p5g_phy[] = {
    { "Digital Block Address",          0x1f, 0x8300 },
    { "enable fiber mode",              0x10, 0x0101 },
    { "Force 2.5G Fiber, 50MHz refclk", 0x18, 0xc010 },
    { "Digital5 Block Address",         0x1f, 0x8340 },
    { "Set os2 mode",                   0x1a, 0x0001 },
    { "BLK0 Block Address",             0x1f, 0x8000 },
    { NULL,                             0x00, 0x0000 }
};

int sgmii_read(phy_dev_t *phy_dev, uint16_t reg, uint16_t *val)
{
    int ret;

    if (reg & 0x8000)
    {
        if ((ret = phy_bus_write(phy_dev, 0x1f, reg & 0xfff0)))
            goto Exit;

        if ((ret = phy_bus_read(phy_dev,  0x10 | (reg & 0x000f), val)))
            goto Exit;

        if ((ret = phy_bus_write(phy_dev, 0x1f, 0x8000)))
            goto Exit;
    }
    else
    {
        if ((ret = phy_bus_read(phy_dev,  reg, val)))
            goto Exit;
    }

Exit:
    return ret;
}

int sgmii_write(phy_dev_t *phy_dev, uint16_t reg, uint16_t val)
{
    int ret;

    if (reg & 0x8000)
    {
        if ((ret = phy_bus_write(phy_dev, 0x1f, reg & 0xfff0)))
            goto Exit;

        if ((ret = phy_bus_write(phy_dev,  0x10 | (reg & 0x000f), val)))
            goto Exit;

        if ((ret = phy_bus_write(phy_dev, 0x1f, 0x8000)))
            goto Exit;
    }
    else
    {
        if ((ret = phy_bus_write(phy_dev,  reg, val)))
            goto Exit;
    }

Exit:
    return ret;
}

int sgmii_read_status(phy_dev_t *phy_dev)
{
    uint16_t val;
    int ret, speed, duplex;

    if(bcm_gpio_get_data(29))
        phy_dev->link = phy_dev->cascade_prev->link;
    else
        phy_dev->link = 0;
    phy_dev->link = phy_dev->cascade_prev->link;
    phy_dev->speed = PHY_SPEED_UNKNOWN;
    phy_dev->duplex = PHY_DUPLEX_UNKNOWN;

    if ((ret = sgmii_read(phy_dev, XGXS_STATUS1, &val)))
        return ret;

    if(!bcm_gpio_get_data(29))
        phy_dev->link = ((val >> 8) & 0x1);

    if (!phy_dev->link)
        return 0;

    speed = val & 0x3;
    duplex = 1; /* ??? */

    if (speed == 0)
        phy_dev->speed = PHY_SPEED_10;
    else if (speed == 1)
        phy_dev->speed = PHY_SPEED_100;
    else if (speed == 2)
        phy_dev->speed = PHY_SPEED_1000;
    else if (speed == 3)
        phy_dev->speed = PHY_SPEED_2500;

    phy_dev->duplex = duplex ? PHY_DUPLEX_FULL : PHY_DUPLEX_HALF;

    return 0;
}

int sgmii_refclk_init(phy_dev_t *phy_dev, sgmii_refclk_t refclk)
{
    if (refclk == SGMII_REFCLK_50)
        phy_dev_prog(phy_dev, refclk50m_vco6p25g);

    if (refclk == SGMII_REFCLK_80)
        phy_dev_prog(phy_dev, refclk80m_vco6p25g);

    if (refclk == SGMII_REFCLK_125)
        phy_dev_prog(phy_dev, refclk125m_vco6p25g);

    return 0;
}

int sgmii_speed_set(phy_dev_t *phy_dev, phy_speed_t speed)
{
    int ret = 0;
    uint16_t bmcr;
    prog_entry_t *prog = NULL;

    switch (speed)
    {
    case PHY_SPEED_UNKNOWN:
    {
        bmcr = 0x1140;
        prog = forced_speed_sgmii_os5; 
        break;
    }
    case PHY_SPEED_10:
    {
        bmcr = 0x0100;
        prog = forced_speed_sgmii_os5; 
        break;
    }
    case PHY_SPEED_100:
    {
        bmcr = 0x2100;
        prog = forced_speed_sgmii_os5; 
        break;
    }
    case PHY_SPEED_1000:
    {
        bmcr = 0x0140;
        prog = forced_speed_sgmii_os5; 
        break;
    }
    case PHY_SPEED_2500:
    {
        bmcr = 0x0140;
        prog = forced_speed_2p5g_phy; 
        break;
    }
    default:
        goto Exit;
    }

    if ((ret = sgmii_write(phy_dev, 0x00, 0x8000)))
        goto Exit;

    udelay(10000);

    phy_dev_prog(phy_dev, disable_pll);
    phy_dev_prog(phy_dev, refclk50m_vco6p25g);
    phy_dev_prog(phy_dev, toggle_pll75);
    phy_dev_prog(phy_dev, disable_afe_limit_amp);
    phy_dev_prog(phy_dev, prog);

    if ((ret = sgmii_write(phy_dev, 0x00, bmcr)))
        goto Exit;

    phy_dev_prog(phy_dev, enable_pll);

    udelay(10000);
    udelay(10000);

Exit:
    return ret;
}
