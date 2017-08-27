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
 *  Created on: Dec 2015
 *      Author: yuval.raviv@broadcom.com
 */

/*
 * Phy driver for 6848 SGMII/HSGMII
 */

#include "bus_drv.h"
#include "phy_drv.h"
#include "phy_drv_mii.h"
#include "rdp_map.h"
#include "pmc_drv.h"
#include <bcm_otp.h>
#include <access_macros.h>
#ifndef _CFE_
#include "wan_drv.h"
#endif

#define XGXS_STATUS1     0x8122

static uint32_t sgmii_enabled;

#pragma pack(push,1)
typedef struct
{
    uint32_t select_sgmii:1;
    uint32_t reserved3:5;
    uint32_t refclk_freq_sel:5;
    uint32_t link_down_tx_dis:1;
    uint32_t serdes_test_en:1;
    uint32_t mdio_st:1;
    uint32_t serdes_devad:5;
    uint32_t serdes_prtad:5;
    uint32_t reserved1:2;
    uint32_t serdes_reset:1;
    uint32_t reset_mdioregs:1;
    uint32_t reset_pll:1;
    uint32_t reserved0:1;
    uint32_t pwrdwn:1;
    uint32_t iddq:1;
} egphy_single_serdes_ctrl_t;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct
{
    uint32_t reserved:22;
    uint32_t apd_state:3;
    uint32_t debounced_signal_detect:1;
    uint32_t pll_lock:1;
    uint32_t sync_status:1;
    uint32_t sgmii:1;
    uint32_t rxseqdone1g:1;
    uint32_t rx_sigdet:1;
    uint32_t link_status:1;
} egphy_single_serdes_stat_t;
#pragma pack(pop)

static prog_entry_t toggle_pll75[] = {
    { "XGXSBlk2 Block Address",         0x1f, 0x8100 },
    { "Override pin, use mdio",         0x16, 0x0020 },
    { "TxPLL Block Address",            0x1f, 0x8050 },
    { "Toggle mmd_resetb to 0",         0x14, 0x8021 },
    { "Toggle mmd_resetb to 1",         0x14, 0x8821 },
    { NULL,                             0x00, 0x0000 }
};

#ifndef _CFE_
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
    { NULL,                             0x00, 0x0000 }
};
#endif

static prog_entry_t forced_speed_1g_sgmii_os5[] = {
    { "BLK0 Block Address",             0x1f, 0x8000 },
    { "disable pll start sequencer",    0x10, 0x0c2f },
    { "Digital Block Address",          0x1f, 0x8300 },
    { "enable sgmii mode",              0x10, 0x0100 }, /* for SFP use value 0x010d */
    { "RX2 Block Address",              0x1f, 0x8470 },
    { "Set step_one[1:0]",              0x13, 0x1251 },
    { "Digital5 Block Address",         0x1f, 0x8340 },
    { "set oversampling mode",          0x1a, 0x0003 },
    { "BLK0 Block Address",             0x1f, 0x8000 },
    { "enable pll start sequencer",     0x10, 0x2c2f },
    { NULL,                             0x00, 0x0000 }
}; 

static prog_entry_t forced_speed_2p5g_sgmii[] = {
    { "BLK0 Block Address",             0x1f, 0x8000 },
    { "disable pll start sequencer",    0x10, 0x0c2f },
    { "Digital Block Address",          0x1f, 0x8300 },
    { "enable fiber mode",              0x10, 0x0105 }, /* Use 0x010d for SMA debug */
    { "Force 2.5G Fiber, 50MHz refclk", 0x18, 0xc010 },
    { "Digital5 Block Address",         0x1f, 0x8340 },
    { "Set os2 mode",                   0x1a, 0x0001 },
    { "BLK0 Block Address",             0x1f, 0x8000 },
    { "disable AN, set 1G mode",        0x00, 0x0140 },
    { "enable pll start sequencer",     0x10, 0x2c2f },
    { NULL,                             0x00, 0x0000 }
};

static prog_entry_t afe_sgmii[] = {
    { "RX3 Slicer",                     0x1f, 0x8480 },
    { "disable lmtcal",                 0x12, 0x83f8 },
    { NULL,                             0x00, 0x0000 }
};

static int sgmii_read(phy_dev_t *phy_dev, uint16_t reg, uint16_t *val)
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

static int sgmii_write(phy_dev_t *phy_dev, uint16_t reg, uint16_t val)
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

static int sgmii_read_status(phy_dev_t *phy_dev)
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

#ifndef _CFE_
#define AFEPLL_PLL_RESETS      0x010
#define AFEPLL_PLL_CFG0        0x014
#define AFEPLL_PLL_CFG1        0x018
#define AFEPLL_PLL_NDIV        0x01c
#define AFEPLL_PLL_PDIV        0x020
#define AFEPLL_PLL_LOOP0       0x024
#define AFEPLL_PLL_LOOP1       0x028
#define AFEPLL_PLL_CH01_CFG    0x02c
#define AFEPLL_PLL_CH23_CFG    0x030
#define AFEPLL_PLL_CH45_CFG    0x034
#define AFEPLL_PLL_OCTRL       0x038
#define AFEPLL_PLL_STAT        0x03c
#define AFEPLL_PLL_STRAP       0x040
#define AFEPLL_PLL_DECNDIV     0x044
#define AFEPLL_PLL_DECPDIV     0x048
#define AFEPLL_PLL_DECCH25     0x04c

static void sgmii_program_pll(uint8_t pdiv, uint16_t ndiv_int,
    uint32_t ndiv_frac, uint8_t mdiv, uint8_t ki, uint8_t ka, uint8_t kp,
    bool min_vco_freq, int is_sgmii_155p2)
{
    uint32_t retry = 10;
    uint32 data;
#define PLL_CTRL_LOW 0x0a040001
#define PLL_CTRL_HIGH 0x00100300
#define AFEPLL_PLL_LOOP0_SS_KI_SHIFT 8
#define AFEPLL_PLL_LOOP0_SS_KP_SHIFT 12

	/* Reset the PLL (reset both resetb and post_resetb) poweron and poweron_ldo */
    ReadBPCMRegister(PMB_ADDR_SYSPLL1, AFEPLL_PLL_RESETS, &data);
	data = (data & 0xfffffffc) | 0x00000014;
    WriteBPCMRegister(PMB_ADDR_SYSPLL1, AFEPLL_PLL_RESETS, data);

    if (!is_sgmii_155p2)
    {
        /* Program the override bit */
        ReadBPCMRegister(PMB_ADDR_SYSPLL1, AFEPLL_PLL_PDIV, &data);
        data = data | 0x80000000;
        WriteBPCMRegister(PMB_ADDR_SYSPLL1, AFEPLL_PLL_PDIV, data);

        /* Program the pdiv, ndiv and frac value */
        ReadBPCMRegister(PMB_ADDR_SYSPLL1, AFEPLL_PLL_PDIV, &data);
        data = data & 0xFFFFFFF0;
        data = data | pdiv;
        data = data | 0x80000000;
        WriteBPCMRegister(PMB_ADDR_SYSPLL1, AFEPLL_PLL_PDIV, data);

        ReadBPCMRegister(PMB_ADDR_SYSPLL1, AFEPLL_PLL_NDIV, &data);
        data = data & 0xFFFFFC00;
        data = data | ndiv_int;
        data = data | 0x80000000;
        WriteBPCMRegister(PMB_ADDR_SYSPLL1, AFEPLL_PLL_NDIV, data);

        /* Override en for the mdiv values */
        ReadBPCMRegister(PMB_ADDR_SYSPLL1, AFEPLL_PLL_CH01_CFG, &data);
        data = data | 0x80008000;
        WriteBPCMRegister(PMB_ADDR_SYSPLL1, AFEPLL_PLL_CH01_CFG, data);

        /* Set new mdiv values */
        ReadBPCMRegister(PMB_ADDR_SYSPLL1, AFEPLL_PLL_CH01_CFG, &data);
        data = data & 0xFF00FF00;
        data = data | mdiv | ( (mdiv+1) << 16); /* 0x000c000a */;
        WriteBPCMRegister(PMB_ADDR_SYSPLL1, AFEPLL_PLL_CH01_CFG, data);

        /* Program the control register */
        ReadBPCMRegister(PMB_ADDR_SYSPLL1, AFEPLL_PLL_CFG0, &data);
        data = data & 0x0;
        data = data | PLL_CTRL_LOW; 
        WriteBPCMRegister(PMB_ADDR_SYSPLL1, AFEPLL_PLL_CFG0, data);

        ReadBPCMRegister(PMB_ADDR_SYSPLL1, AFEPLL_PLL_CFG1, &data);
        data = data & 0x0;
        data = data | PLL_CTRL_HIGH; 
        WriteBPCMRegister(PMB_ADDR_SYSPLL1, AFEPLL_PLL_CFG1, data);

        /* Program the KI,KA,KP values */
        ReadBPCMRegister(PMB_ADDR_SYSPLL1, AFEPLL_PLL_LOOP0, &data);
        data = data & 0xFFFF088F;
        data = data | ( ( ki << AFEPLL_PLL_LOOP0_SS_KI_SHIFT) | ( kp << AFEPLL_PLL_LOOP0_SS_KP_SHIFT) ); 
        WriteBPCMRegister(PMB_ADDR_SYSPLL1, AFEPLL_PLL_LOOP0, data);
    }

    /* Deassert the i_resetb of PLL */
    ReadBPCMRegister(PMB_ADDR_SYSPLL1, AFEPLL_PLL_RESETS, &data);
    data = data | 0x1;
    WriteBPCMRegister(PMB_ADDR_SYSPLL1, AFEPLL_PLL_RESETS, data);

	do
	{
        printk("... Waiting for PLL lock .. \n");
        ReadBPCMRegister(PMB_ADDR_SYSPLL1, AFEPLL_PLL_STAT, &data);
		data &= 0x00002000;
        if (data)
            break;
	 	udelay(1000000);
	}
	while (retry--);
	
	/* Deassert the i_post_resetb of PLL */
    ReadBPCMRegister(PMB_ADDR_SYSPLL1, AFEPLL_PLL_RESETS, &data);
	data |= 0x2;
    WriteBPCMRegister(PMB_ADDR_SYSPLL1, AFEPLL_PLL_RESETS, data);

    udelay(2);
}

static void sgmii_refclk_init(phy_dev_t *phy_dev)
{
    serdes_wan_type_t type = wan_serdes_type_get();

    if (type == SERDES_WAN_TYPE_NONE)
        return;

    if (type == SERDES_WAN_TYPE_GPON)
    {
        phy_dev_prog(phy_dev, refclk80m_vco6p25g);
        sgmii_program_pll(6, 125, 0, 27, 2, 0 ,9, 1, 1);
    }
    else
    {
        phy_dev_prog(phy_dev, refclk125m_vco6p25g);
        sgmii_program_pll(2, 50, 0, 25, 2, 0, 9, 1, 0);
    }
}
#endif

static int sgmii_init(phy_dev_t *phy_dev)
{
    int ret;

    if ((ret = mii_init(phy_dev)))
        return ret;

    phy_dev_prog(phy_dev, toggle_pll75);
    phy_dev_prog(phy_dev, afe_sgmii);

    if (phy_dev->mii_type == PHY_MII_TYPE_HSGMII)
        phy_dev_prog(phy_dev, forced_speed_2p5g_sgmii);
    else
        phy_dev_prog(phy_dev, forced_speed_1g_sgmii_os5);

#ifndef _CFE_
    /* SGMII SyncE reference clock */
    sgmii_refclk_init(phy_dev);
#endif

    return 0;
}

static int sgmii_power_on(void)
{
    int ret;

    if ((ret = PowerOnDevice(PMB_ADDR_SGMII)))
        printk("Failed to PowerOnDevice PMB_ADDR_SGMII block\n");
    else
        udelay(100);

    return ret;
}

static void sgmii_mux_config(void)
{
    uint32_t data = 0x01;

    /* Mux EMAC0 to SGMII */
    WRITE_32(EGPHY_RGMII_0_MUX_CONFIG, data);
    udelay(50);
}

static void sgmii_serdes_cntrl_init(void)
{
    egphy_single_serdes_ctrl_t serdes_ctrl;

    READ_32(EGPHY_SINGLE_SERDES_CNTRL, serdes_ctrl);

    serdes_ctrl.select_sgmii = 1;
    WRITE_32(EGPHY_SINGLE_SERDES_CNTRL, serdes_ctrl);
    udelay(50);

    serdes_ctrl.iddq = 0;
    serdes_ctrl.pwrdwn = 0;
    WRITE_32(EGPHY_SINGLE_SERDES_CNTRL, serdes_ctrl);
    udelay(1000);

    serdes_ctrl.serdes_reset = 0;
    WRITE_32(EGPHY_SINGLE_SERDES_CNTRL, serdes_ctrl);
    udelay(50);

    serdes_ctrl.reset_mdioregs = 0;
    WRITE_32(EGPHY_SINGLE_SERDES_CNTRL, serdes_ctrl);
    udelay(50);

    serdes_ctrl.reset_pll = 0;
    WRITE_32(EGPHY_SINGLE_SERDES_CNTRL, serdes_ctrl);
    udelay(50);

    serdes_ctrl.serdes_prtad = 0x6;
    WRITE_32(EGPHY_SINGLE_SERDES_CNTRL, serdes_ctrl);
    udelay(50);
}

static void wait_for_serdes_ready(void)
{
    uint32_t retry = 2000;
    egphy_single_serdes_stat_t serdes_stat;

    /* Wait till pll_lock and link_status become '1' */
    do {
        READ_32(EGPHY_SINGLE_SERDES_STAT, serdes_stat);
        if (serdes_stat.pll_lock) /* && serdes_stat.link_status */
            break;
        udelay(10);
    } while (--retry);

    if (!retry)
    {
        printk("SGMII Error: wait_for_serdes_ready() reached maximum retries. pll_lock=%d link_status=%d\n",
            serdes_stat.pll_lock, serdes_stat.link_status);
    }
}

static int sgmii_cfg(void)
{
    unsigned int val;
    int rc = bcm_otp_get_row(8, &val);
    int sgmii_disabled = val & 1 << 25;

    if (rc)
        printk("Cannot read SGMII OTP (rc=%d)\n", rc);

    if (rc || sgmii_disabled || sgmii_power_on())
        return -1;

    sgmii_mux_config();
    sgmii_serdes_cntrl_init();
    wait_for_serdes_ready();

    return 0;
}

static int sgmii_dev_add(phy_dev_t *phy_dev)
{
    sgmii_enabled++;

    return 0;
}

static int sgmii_dev_del(phy_dev_t *phy_dev)
{
    sgmii_enabled--;

    return 0;
}

static int sgmii_drv_init(phy_drv_t *phy_drv)
{
    if (sgmii_enabled)
        return sgmii_cfg();

    phy_drv->initialized = 1;

    return 0;
}

phy_drv_t phy_drv_6848_sgmii =
{
    .phy_type = PHY_TYPE_6848_SGMII,
    .name = "SGMII",
    .read = sgmii_read,
    .write = sgmii_write,
    .power_set = mii_power_set,
    .read_status = sgmii_read_status,
    .init = sgmii_init,
    .dev_add = sgmii_dev_add,
    .dev_del = sgmii_dev_del,
    .drv_init = sgmii_drv_init,
};
