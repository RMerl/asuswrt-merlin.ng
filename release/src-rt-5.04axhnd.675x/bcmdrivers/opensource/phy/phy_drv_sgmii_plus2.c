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
#include "linux/delay.h"
#include "access_macros.h"
#include "dt_access.h"
#include "pmc_sgmii.h"

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

    phy_dev->link = 0;
    phy_dev->speed = PHY_SPEED_UNKNOWN;
    phy_dev->duplex = PHY_DUPLEX_UNKNOWN;

    if ((ret = sgmii_read(phy_dev, XGXS_STATUS1, &val)))
        return ret;

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

    mdelay(10);

    phy_dev_prog(phy_dev, disable_pll);
    phy_dev_prog(phy_dev, refclk50m_vco6p25g);
    phy_dev_prog(phy_dev, toggle_pll75);
    phy_dev_prog(phy_dev, disable_afe_limit_amp);
    phy_dev_prog(phy_dev, prog);

    if ((ret = sgmii_write(phy_dev, 0x00, bmcr)))
        goto Exit;

    phy_dev_prog(phy_dev, enable_pll);

    mdelay(20);

Exit:
    return ret;
}

static void __iomem *sgmii_base;

static int sgmii_probe(dt_device_t *pdev)
{
    int ret;

    sgmii_base = dt_dev_remap_resource(pdev, 0);
    if (IS_ERR(sgmii_base))
    {
        ret = PTR_ERR(sgmii_base);
        sgmii_base = NULL;
        dev_err(&pdev->dev, "Missing sgmii_base entry\n");
        goto Exit;
    }

    dev_dbg(&pdev->dev, "sgmii_base=0x%px\n", sgmii_base);
    dev_info(&pdev->dev, "registered\n");

    return 0;

Exit:
    return ret;
}

static const struct of_device_id of_platform_table[] = {
    { .compatible = "brcm,sgmii" },
    { /* end of list */ },
};

static struct platform_driver of_platform_driver = {
    .driver = {
        .name = "brcm-sgmii",
        .of_match_table = of_platform_table,
    },
    .probe = sgmii_probe,
};
module_platform_driver(of_platform_driver);

#define SGMII_BASE      (void *)sgmii_base

#define SGMII_CTRL      SGMII_BASE + 0x0004
#define SGMII_STAT      SGMII_BASE + 0x0008

int ReadBPCMRegister(int devAddr, int wordOffset, uint32_t * value);
int WriteBPCMRegister(int devAddr, int wordOffset, uint32_t value);

static uint32_t phy_addr;

#pragma pack(push,1)
typedef struct
{
    uint32_t IDDQ:1;
    uint32_t PWRDWN:1;
    uint32_t Reserved0:1;
    uint32_t RESET_PLL:1;
    uint32_t RESET_MDIOREGS:1;
    uint32_t SERDES_RESET:1;
    uint32_t Reserved1:2;
    uint32_t SERDES_PRTAD:5;
    uint32_t SERDES_DEVAD:5;
    uint32_t MDIO_ST:1;
    uint32_t SERDES_TEST_EN:1;
    uint32_t LINK_DOWN_TX_DIS:1;
    uint32_t SERDES_PRTAD_BCST:5;
    uint32_t Reserved2:6;
} sgmii_ctrl_t;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct
{
    uint32_t LINK_STATUS:1;
    uint32_t RX_SIGDET:1;
    uint32_t RXSEQDONE1G:1;
    uint32_t SGMII:1;
    uint32_t SYNC_STATUS:1;
    uint32_t PLL_LOCK:1;
    uint32_t DEB_SIG_DETECT:1;
    uint32_t APD_STATE:3;
    uint32_t Reserved:22;
} sgmii_stat_t;
#pragma pack(pop)

static int _phy_speed_set(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex)
{
    int ret;

    if ((ret = sgmii_speed_set(phy_dev, speed)))
        goto Exit;

Exit:
    return ret;
}

static int _phy_init(phy_dev_t *phy_dev)
{
    int ret;
    phy_speed_t speed;;

    if (phy_dev->mii_type == PHY_MII_TYPE_HSGMII)
        speed = PHY_SPEED_2500;
    else
        speed = PHY_SPEED_1000;

    if ((ret = sgmii_speed_set(phy_dev, speed)))
        return ret;

    return 0;
}

static void sgmii_ctrl_init(void)
{
    sgmii_ctrl_t sgmii_ctrl;

    sgmii_ctrl.SERDES_PRTAD = phy_addr;
    WRITE_32(SGMII_CTRL, sgmii_ctrl);
    udelay(1000);

    READ_32(SGMII_CTRL, sgmii_ctrl);
    sgmii_ctrl.RESET_PLL = 1;
    sgmii_ctrl.RESET_MDIOREGS = 1;
    sgmii_ctrl.SERDES_RESET = 1;
    sgmii_ctrl.IDDQ = 0;
    sgmii_ctrl.PWRDWN = 0;
    WRITE_32(SGMII_CTRL, sgmii_ctrl);
    udelay(1000);

    sgmii_ctrl.RESET_PLL = 0;
    sgmii_ctrl.RESET_MDIOREGS = 0;
    sgmii_ctrl.SERDES_RESET = 0;
    WRITE_32(SGMII_CTRL, sgmii_ctrl);
    udelay(1000);
}
static void wait_for_pll_lock(void)
{
    uint32_t retry = 20;
    sgmii_stat_t sgmii_stat;

    do {
        READ_32(SGMII_STAT, sgmii_stat);
        if (sgmii_stat.PLL_LOCK)
            break;
        udelay(1000);
    } while (--retry);

    if (!retry)
        printk("SGMII Error: wait_for_pll_lock() reached maximum retries\n");
    else
        printk("SGMII PLL locked\n");
}

static int sgmii_cfg(void)
{
    sgmii_bpcm_init();
    sgmii_ctrl_init();
    wait_for_pll_lock();

    return 0;
}

static int _phy_dev_add(phy_dev_t *phy_dev)
{
    phy_addr = phy_dev->addr;

    return 0;
}

static int _phy_dev_del(phy_dev_t *phy_dev)
{
    phy_addr = 0;

    return 0;
}
static int _phy_drv_init(phy_drv_t *phy_drv)
{
    if (phy_addr && sgmii_cfg())
    {
        printk("Failed to initialize the sgmii driver\n");
        return -1;
    }

    phy_drv->initialized = 1;

    return 0;
}

static int _phy_caps_get(phy_dev_t *phy_dev, int caps_type,  uint32_t *pcaps)
{
    *pcaps = PHY_CAP_AUTONEG | PHY_CAP_10_FULL | PHY_CAP_100_FULL | PHY_CAP_1000_FULL | PHY_CAP_2500;

    return 0;
}

static int _phy_caps_set(phy_dev_t *phy_dev, uint32_t caps)
{
    return 0;
}

static int _phy_inter_phy_types_get(phy_dev_t *phy_dev, inter_phy_types_dir_t if_dir, uint32_t *inter_phy_types)
{
    *inter_phy_types = INTER_PHY_TYPES_S1K2K_M;
    phy_dev->inter_phy_types = *inter_phy_types;

    return 0;
}

phy_drv_t phy_drv_sgmii =
{
    .phy_type = PHY_TYPE_SGMII,
    .name = "SGMII",
    .read = sgmii_read,
    .write = sgmii_write,
    .power_get = mii_power_get,
    .power_set = mii_power_set,
    .read_status = sgmii_read_status,
    .speed_set = _phy_speed_set,
    .caps_get = _phy_caps_get,
    .caps_set = _phy_caps_set,
    .phyid_get = mii_phyid_get,
    .init = _phy_init,
    .dev_add = _phy_dev_add,
    .dev_del = _phy_dev_del,
    .drv_init = _phy_drv_init,
    .inter_phy_types_get = _phy_inter_phy_types_get,
};

