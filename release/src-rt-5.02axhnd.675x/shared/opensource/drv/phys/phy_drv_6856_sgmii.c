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
 * Phy driver for 6856 SGMII/HSGMII
 */

#include "bus_drv.h"
#include "phy_drv.h"
#include "phy_drv_mii.h"
#include "phy_drv_sgmii_plus2.h"
#include "pmc_drv.h"
#include "access_macros.h"
#include "bcm_map_part.h"

#define SGMII_CTRL      SGMII_BASE + 0x0004
#define SGMII_STAT      SGMII_BASE + 0x0008

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
        speed = PHY_SPEED_1000;

    if ((ret = sgmii_speed_set(phy_dev, speed)))
        return ret;

    return 0;
}

static int sgmii_bpcm_init(void)
{
    uint32_t data;

    ReadBPCMRegister(PMB_ADDR_WAN, 0x10, &data);
    data |= (1 << 5); // sgmii_z2_serdes_reset_n
    WriteBPCMRegister(PMB_ADDR_WAN, 0x10, data);

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

phy_drv_t phy_drv_6856_sgmii =
{
    .phy_type = PHY_TYPE_6856_SGMII,
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
