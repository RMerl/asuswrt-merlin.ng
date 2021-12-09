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
 * Phy driver for 6838/6848 internal dual/quad 1G phy
 */

#include "bus_drv.h"
#include "phy_drv.h"
#include "phy_drv_mii.h"
#include "phy_drv_brcm.h"
#include "rdp_map.h"
#include "access_macros.h"

#define CORE_BASE1E             0x000e /* Test1_Register */
#define CORE_EXPB0              0x00b0 /* Bias Control 0 */
#define DSP_TAP10               0x0125 /* PLL Bandwidth Control */
#define AFE_RXCONFIG_1          0x01e1 /* AFE RXCONFIG 1 */
#define CORE_SHD18_000          0x0028 /* Auxiliary Control */
#define CORE_SHD1C_0D			0x001d /* LED Selector 1 */

#define CORE_SHD_BICOLOR_LED0   0x00aa

static uint32_t enabled_ports;

#pragma pack(push,1)
typedef struct
{
    uint32_t reserved:6;
    uint32_t gphy_testmux_sw_init:1;
    uint32_t gphy_pll_ref_clk_sel:2;
    uint32_t phy_pll_clk_sel_div5:2;
    uint32_t phy_sel:2;
    uint32_t phy_test_mode:4;
    uint32_t PHY_TEST_EN:1;
    uint32_t PHYA:5;
    uint32_t GPHY_CK25_DIS:1;
    uint32_t IDDQ_BIAS:1;
    uint32_t DLL_EN:1;
    uint32_t PWRDWN:4;
    uint32_t iddq_global_pwr:1;
    uint32_t RST:1;
} egphy_gphy_out_reg_t;
#pragma pack(pop)

#ifdef CONFIG_BCM96848
static int _phy_afe(phy_dev_t *phy_dev)
{
    int ret;

    /* Enable the dsp clock */
    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | CORE_SHD18_000, 0x0c30)))
        goto Exit;

    /* AFE RXCONFIG 1 */
    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | AFE_RXCONFIG_1, 0x9b2f)))
        goto Exit;

    /* Force trim overwrite and set I_ext trim to 0000 */
    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | CORE_BASE1E, 0x0010)))
        goto Exit;

    /* Adjust bias current trim (+0% swing, +0 tick) */
    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | DSP_TAP10, 0x011b)))
        goto Exit;

    /* Reset R_CAL/RC_CAL Engine */
    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | CORE_EXPB0, 0x0010)))
        goto Exit;

    /* Disable Reset R_CAL/RC_CAL Engine */
    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | CORE_EXPB0, 0x0000)))
        goto Exit;

Exit:
    return ret;
}
#endif

static int _phy_init(phy_dev_t *phy_dev)
{
    int ret;

    if ((ret = mii_init(phy_dev)))
        goto Exit;

    /* LED Selector: set LEDs to bicolor mode  */
    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | CORE_SHD1C_0D, CORE_SHD_BICOLOR_LED0)))
        goto Exit;

#ifdef CONFIG_BCM96848
    /* AFE workaround */
    if ((ret = _phy_afe(phy_dev)))
        goto Exit;
#endif

    if ((ret = brcm_egphy_force_auto_mdix_set(phy_dev, 1)))
        goto Exit;

    if ((ret = brcm_egphy_eth_wirespeed_set(phy_dev, 1)))
        goto Exit;

Exit:
    return ret;
}

static int _phy_cfg(uint32_t port_map)
{
    egphy_gphy_out_reg_t gphy_out;

    READ_32(EGPHY_UBUS_MISC_EGPHY_GPHY_OUT, gphy_out);

    gphy_out.IDDQ_BIAS = 0;
    gphy_out.PWRDWN = 0;
    gphy_out.DLL_EN = 0;
    gphy_out.iddq_global_pwr = 0;
    gphy_out.GPHY_CK25_DIS = 0;
    gphy_out.RST = 0;
    gphy_out.PHYA = 0;

    WRITE_32(EGPHY_UBUS_MISC_EGPHY_GPHY_OUT, gphy_out);
    udelay(50);

    gphy_out.RST = 1;
    gphy_out.PHYA = 1;
    gphy_out.PWRDWN = ~port_map;
    gphy_out.iddq_global_pwr = port_map ? 0 : 1;
    gphy_out.IDDQ_BIAS = port_map ? 0 : 1;

    WRITE_32(EGPHY_UBUS_MISC_EGPHY_GPHY_OUT, gphy_out);
    udelay(50);

    return 0;
}

static int _phy_dev_add(phy_dev_t *phy_dev)
{
    uint32_t port = (uint32_t)phy_dev->priv;

    enabled_ports |= (1 << port);

    return 0;
}

static int _phy_dev_del(phy_dev_t *phy_dev)
{
    uint32_t port = (uint32_t)phy_dev->priv;

    enabled_ports &= ~(1 << port);

    return 0;
}

static int _phy_drv_init(phy_drv_t *phy_drv)
{
    if (_phy_cfg(enabled_ports & 0xf))
    {
        printk("Failed to initialize the egphy driver\n");
        return -1;
    }

    phy_drv->initialized = 1;

    return 0;
}

phy_drv_t phy_drv_6848_egphy =
{
    .phy_type = PHY_TYPE_6848_EGPHY,
    .name = "EGPHY",
    .read = brcm_egphy_read,
    .write = brcm_egphy_write,
    .power_get = mii_power_get,
    .power_set = mii_power_set,
    .apd_get = brcm_egphy_apd_get,
    .apd_set = brcm_egphy_apd_set,
    .eee_get = brcm_egphy_eee_get,
    .eee_set = brcm_egphy_eee_set,
    .eee_resolution_get = brcm_egphy_eee_resolution_get,
    .read_status = brcm_read_status,
    .speed_set = mii_speed_set,
    .caps_get = mii_caps_get,
    .caps_set = mii_caps_set,
    .phyid_get = mii_phyid_get,
    .auto_mdix_set = brcm_egphy_force_auto_mdix_set,
    .auto_mdix_get = brcm_egphy_force_auto_mdix_get,
    .init = _phy_init,
    .dev_add = _phy_dev_add,
    .dev_del = _phy_dev_del,
    .drv_init = _phy_drv_init,
};
