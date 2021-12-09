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
#include "phy_drv_sgmii_plus2.h"
#include "rdp_map.h"
#include "pmc_drv.h"
#include <bcm_otp.h>
#include <access_macros.h>
#ifndef _CFE_
#include "wan_drv.h"
#endif

static uint32_t phy_addr;

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

static void _refclk_init(phy_dev_t *phy_dev)
{
    serdes_wan_type_t type = wan_serdes_type_get();

    if (type == SERDES_WAN_TYPE_NONE)
        return;

    if (type == SERDES_WAN_TYPE_GPON)
    {
        sgmii_refclk_init(phy_dev, SGMII_REFCLK_80);
        sgmii_program_pll(6, 125, 0, 27, 2, 0 ,9, 1, 1);
    }
    else
    {
        sgmii_refclk_init(phy_dev, SGMII_REFCLK_125);
        sgmii_program_pll(2, 50, 0, 25, 2, 0, 9, 1, 0);
    }
}
#endif

static int _phy_speed_set(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex)
{
    int ret;

    if (speed != PHY_SPEED_2500)
        speed = PHY_SPEED_UNKNOWN;

    if ((ret = sgmii_speed_set(phy_dev, speed)))
        goto Exit;

    phy_dev_read_status(phy_dev);

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
        speed = PHY_SPEED_UNKNOWN;

    if ((ret = sgmii_speed_set(phy_dev, speed)))
        return ret;

#ifndef _CFE_
    /* SGMII SyncE reference clock */
    _refclk_init(phy_dev);
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

static void sgmii_ctrl_init(void)
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

    serdes_ctrl.serdes_prtad = phy_addr;
    WRITE_32(EGPHY_SINGLE_SERDES_CNTRL, serdes_ctrl);
    udelay(50);
}

static void wait_for_pll_lock(void)
{
    uint32_t retry = 20;
    egphy_single_serdes_stat_t sgmii_stat;

    do {
        READ_32(EGPHY_SINGLE_SERDES_STAT, sgmii_stat);
        if (sgmii_stat.pll_lock)
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
    unsigned int val;
    int rc = bcm_otp_get_row(8, &val);
    int sgmii_disabled = val & 1 << 25;

    if (rc)
        printk("Cannot read SGMII OTP (rc=%d)\n", rc);

    if (rc || sgmii_disabled || sgmii_power_on())
        return -1;

    sgmii_mux_config();
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

phy_drv_t phy_drv_6848_sgmii =
{
    .phy_type = PHY_TYPE_6848_SGMII,
    .name = "SGMII",
    .read = sgmii_read,
    .write = sgmii_write,
    .power_get = mii_power_get,
    .power_set = mii_power_set,
    .read_status = sgmii_read_status,
    .speed_set = _phy_speed_set,
    .phyid_get = mii_phyid_get,
    .init = _phy_init,
    .dev_add = _phy_dev_add,
    .dev_del = _phy_dev_del,
    .drv_init = _phy_drv_init,
};
