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
 * Phy driver for 6846, 6856 internal quad 1G phy
 */

#include "bus_drv.h"
#include "phy_drv.h"
#include "phy_drv_mii.h"
#include "phy_drv_brcm.h"
#include "access_macros.h"
#include "xrdp_led_init.h"
#include "dt_access.h"

static uintptr_t qegphy_base;
static uint16_t base_addr = 1;

static int egphy_probe(dt_device_t *pdev)
{
    int ret;
    dt_handle_t node = dt_dev_get_handle(pdev);

    qegphy_base = dt_dev_remap_resource(pdev, 0);
    if (IS_ERR(qegphy_base))
    {
        ret = PTR_ERR(qegphy_base);
        qegphy_base = NULL;
        dev_err(&pdev->dev, "Missing qegphy_base entry\n");
        goto Exit;
    }

    base_addr = dt_property_read_u32_default(node, "base-addr", base_addr);

    dev_dbg(&pdev->dev, "qegphy_base=0x%lx\n", qegphy_base);
    dev_dbg(&pdev->dev, "base_addr=%d\n", base_addr);
    dev_info(&pdev->dev, "registered\n");

    return 0;

Exit:
    return ret;
}

static const struct of_device_id of_platform_table[] = {
    { .compatible = "brcm,egphy" },
    { /* end of list */ },
};

static struct platform_driver of_platform_driver = {
    .driver = {
        .name = "brcm-egphy",
        .of_match_table = of_platform_table,
    },
    .probe = egphy_probe,
};
module_platform_driver(of_platform_driver);

#define QEGPHY_BASE             (void *)qegphy_base
#define QEGPHY_CTRL_REG         QEGPHY_BASE + 0x0008

#define CORE_SHD1C_02			0x0012 /* LED Control0 */
#define CORE_SHD1C_09			0x0019 /* LED Control */
#define CORE_SHD1C_0D			0x001d /* LED Selector 1 */
#define CORE_SHD1C_0E			0x001e /* LED Selector 2 */
#define CORE_EXP04              0x0034 /* Bicollor Led Selector */

static uint32_t enabled_addrs = 0;

#pragma pack(push,1)
typedef struct
{
    uint32_t IDDQ_BIAS:1;
    uint32_t EXT_PWR_DOWN:4;
    uint32_t FORCE_DLL_EN:1;
    uint32_t IDDQ_GLOBAL_PWR:1;
    uint32_t CK25_DIS:1;
    uint32_t PHY_RESET:1;
    uint32_t Reserved0:3;
    uint32_t PHY_PHYAD:5;
    uint32_t PLL_REFCLK_SEL:2;
    uint32_t PLL_SEL_DIV5:2;
    uint32_t PLL_CLK125_250_SEL:1;
    uint32_t Reserved1:10;
} qegphy_ctrl_t;
#pragma pack(pop)


#define PHY_ID_51F1_MASK 0x51F1
#define PHY_ID_5321_MASK 0x5321
#define PHY_ID_5371_MASK 0x5371
#define PHY_ID_MASK 0xffff

#define CORE_SHD18_000          0x0028 /* Auxiliary Control Register */
#define CORE_EXPB0              0x00b0 /* Bias Control 0 */
#define DSP_TAP10               0x0125 /* PLL Bandwidth Control */
#define DSP_TAP33_C2            0x0152 /* EEE LPI Timers */
#define DSP_TAP34_C2            0x0156 /* EEE 100TX Mode BW control */
#define DSP_FFE_ZERO_DET_CTL    0x0166 /* FFE Zero Detection Control */
#define AFE_TX_IQ_RX_LP         0x01e4 /* AFE_TX_IQ_RX_LP */
#define AFE_TX_CONFIG_0         0x01e5 /* AFE_TX_CONFIG_0 */
#define AFE_TX_CONFIG_1         0x01ea /* AFE_TX_CONFIG_1 */
#define AFE_TX_CONFIG_2         0x01eb /* AFE_TX_CONFIG_2 */
#define AFE_TEMPSEN_OTHERS      0x01ec /* AFE_TEMPSEN_OTHERS */

static int _phy_afe(phy_dev_t *phy_dev)
{
    int ret;

    /* Enable the DSP clock */
    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | CORE_SHD18_000, 0x0c30)))
        goto Exit;

    /* Turn off AOF */
    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | AFE_TX_CONFIG_0, 0x0000)))
        goto Exit;

    /* 1g AB symmetry Iq */
    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | AFE_TX_CONFIG_1, 0x0bcc)))
        goto Exit;

#ifdef CONFIG_BCM96855
    /* 10BT no change(net increase +2.2%) and 100BT decrease by -4.8% (net increase -2.6%) */
    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | AFE_TX_CONFIG_2, 0x005e)))
        goto Exit;
#endif

    /* LPF BW */
    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | AFE_TX_IQ_RX_LP, 0x233f)))
        goto Exit;

    /* RCAL +6LSB to make impedance from 112 to 100ohm */
    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | AFE_TEMPSEN_OTHERS, 0xad40)))
        goto Exit;

#ifdef CONFIG_BCM96855
    /* since rcal make R smaller, make master current +2.2% */
    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | DSP_TAP10, 0x211b)))
        goto Exit;
#else
    /* since rcal make R smaller, make master current -4% */
    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | DSP_TAP10, 0x091b)))
        goto Exit;
#endif

    /* rx_on_tune 8 -> 0xf */
    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | DSP_TAP33_C2, 0x87f6)))
        goto Exit;

    /* From EEE excel config file for Vitesse fix */
    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | DSP_TAP34_C2, 0x017d)))
        goto Exit;

    /* Enable ffe zero det for Vitesse interop */
    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | DSP_FFE_ZERO_DET_CTL, 0x0015)))
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

static int _phy_init(phy_dev_t *phy_dev)
{
    int ret;

    if ((ret = mii_init(phy_dev)))
        goto Exit;

    /* AFE workaround */
    if ((ret = _phy_afe(phy_dev)))
        goto Exit;
    
    if ((ret = brcm_egphy_force_auto_mdix_set(phy_dev, 1)))
        goto Exit;

    if ((ret = brcm_egphy_eth_wirespeed_set(phy_dev, 1)))
        goto Exit;

Exit:
    return ret;
}

static int _phy_cfg(uint32_t port_map)
{
    qegphy_ctrl_t qegphy_ctrl;

    READ_32(QEGPHY_CTRL_REG, qegphy_ctrl);

    /* Assert reset_n (active low) */
    qegphy_ctrl.PHY_RESET = 1;
    WRITE_32(QEGPHY_CTRL_REG, qegphy_ctrl);
    udelay(1000);

    /* Set GPHY's pll_clk125_250_sel to 250MHz */
    qegphy_ctrl.PLL_CLK125_250_SEL = 1;
    WRITE_32(QEGPHY_CTRL_REG, qegphy_ctrl);
    udelay(1000);

    /* Deassert iddq_global_pwr and iddq_bias */
    qegphy_ctrl.IDDQ_GLOBAL_PWR = 0;
    qegphy_ctrl.IDDQ_BIAS = 0;
    WRITE_32(QEGPHY_CTRL_REG, qegphy_ctrl);
    udelay(1000);

    if (!port_map)
    {
        /* Assert iddq_global_pwr and iddq_bias */
        qegphy_ctrl.IDDQ_GLOBAL_PWR = 1;
        qegphy_ctrl.IDDQ_BIAS = 1;
        WRITE_32(QEGPHY_CTRL_REG, qegphy_ctrl);
        udelay(1000);

        /* Deassert reset_n */
        qegphy_ctrl.PHY_RESET = 0;
        WRITE_32(QEGPHY_CTRL_REG, qegphy_ctrl);
        udelay(1000);

       /* Set GPHY's pll_clk125_250_sel to 125MHz */
        qegphy_ctrl.PLL_CLK125_250_SEL = 0;
        WRITE_32(QEGPHY_CTRL_REG, qegphy_ctrl);
        udelay(1000);

        /* Assert reset_n (active low) */
        qegphy_ctrl.PHY_RESET = 1;
        WRITE_32(QEGPHY_CTRL_REG, qegphy_ctrl);
        udelay(1000);

        return 0;
    }

    /* Set QGPHY base address */
    qegphy_ctrl.PHY_PHYAD = base_addr;

    /* Power only enabled ports */
    qegphy_ctrl.EXT_PWR_DOWN = ~port_map;
    WRITE_32(QEGPHY_CTRL_REG, qegphy_ctrl);
    udelay(1000);

    /* Set GPHY's pll_clk125_250_sel to 125MHz */
    qegphy_ctrl.PLL_CLK125_250_SEL = 0;
    WRITE_32(QEGPHY_CTRL_REG, qegphy_ctrl);

    /* Deassert reset_n */
    qegphy_ctrl.PHY_RESET = 0;
    WRITE_32(QEGPHY_CTRL_REG, qegphy_ctrl);
    udelay(1000);

    return 0;
}

extern phy_drv_t phy_drv_6846_egphy;

static int _phy_afe_all(void)
{
    int i, ret = 0;
    phy_dev_t phy_dev = {};
    phy_dev.phy_drv = &phy_drv_6846_egphy;

    for (i = 0; i < 4; i++)
    {
        phy_dev.addr = base_addr + i;
        ret |= _phy_afe(&phy_dev);
    }

    return ret;
}

static int _phy_dev_add(phy_dev_t *phy_dev)
{
    enabled_addrs |= (1 << phy_dev->addr);

    return 0;
}

static int _phy_dev_del(phy_dev_t *phy_dev)
{
    enabled_addrs &= ~(1 << phy_dev->addr);

    return 0;
}

static int _phy_drv_init(phy_drv_t *phy_drv)
{
    if (_phy_cfg((enabled_addrs >> base_addr) & 0xf))
    {
        printk("Failed to initialize the egphy driver\n");
        return -1;
    }

    if (_phy_afe_all())
    {
        printk("Failed to initialize the phy AFE settings\n");
        return -1;
    }

    phy_drv->initialized = 1;

    return 0;
}

#include "bcm_bca_leds_dt_bindings.h"
#include "bcm_bca_leds.h"

#define MAX_LEDS_PER_PORT 2

static int _phy_leds_init_51XX(phy_dev_t *phy_dev, void *_leds_info, uint8_t is_shifted)
{
    bca_leds_info_t *leds_info = (bca_leds_info_t *)_leds_info;
    int ret = 0;
    int j;
    uint16_t led_shd1c_09 = 0;
    uint16_t led_shd1c_02 = 0;
    uint16_t led_core_exp_04 = 0;
    uint16_t led_shd1c_0d = 0, led_shd1c_0e = 0;

    if (((leds_info->link[0] == LED_SPEED_GBE || leds_info->link[0] == LED_SPEED_ALL) &&
        leds_info->link[0] == leds_info->activity[0]) &&
        ((leds_info->link[1] == LED_SPEED_1G) && leds_info->activity[1] == 0))
    {
        led_shd1c_02 = 0x206;
        led_shd1c_09 = 0;
        led_shd1c_0d = 0xaa;
        led_shd1c_0e = 0x00;
        led_core_exp_04 = 0x102;
    }
    else if(((leds_info->link[1] == LED_SPEED_GBE || leds_info->link[1] == LED_SPEED_ALL) &&
        leds_info->link[1] == leds_info->activity[1]) &&
        ((leds_info->link[0] == LED_SPEED_1G) && leds_info->activity[0] == 0))
    {
        led_shd1c_02 = 0x206;
        led_shd1c_09 = 0;
        led_shd1c_0d = 0x00;
        led_shd1c_0e = 0xaa;
        led_core_exp_04 = 0x120;
    }
    else 
    {

        for (j = 0; j < MAX_LEDS_PER_PORT; j++)
        {
            uint32_t led_mux = leds_info->link[j];
            uint32_t led_activity = leds_info->activity[j];
            uint32_t val = 0;

            if (led_mux == led_activity)
            {
                if (led_mux == LED_SPEED_ALL || led_mux == LED_SPEED_GBE)
                {
                    val = 0x18;
                }
                else if (led_mux == (LED_SPEED_1G | LED_SPEED_100))
                {
                    val = 0x108;
                }
                else if (led_mux == (LED_SPEED_1G | LED_SPEED_10))
                {
                    val = 0x108;
                }
                else if (led_mux == LED_SPEED_1G)
                {
                    val = 0x118;
                }
                else if (led_mux == LED_SPEED_100)
                {
                    val = 0x118;
                }
                else if (led_mux == LED_SPEED_10)
                {
                    val = 0x118;
                }
                else if (led_mux == (LED_SPEED_100 | LED_SPEED_10))
                {
                    val = 0x118;
                }

                if( val > led_shd1c_09)
                    led_shd1c_09 = val;
            }
        }

        for (j = 0; j < MAX_LEDS_PER_PORT; j++)
        {
            uint16_t led_sel = 0;
            uint32_t led_mux = leds_info->link[j];
            uint32_t led_activity = leds_info->activity[j];
            uint16_t val, val2;

            if (led_mux == led_activity)
            {
                if (led_mux == LED_SPEED_ALL || led_mux == LED_SPEED_GBE)
                {
                    val = 0x3;
                    if (led_shd1c_09 == 0x118)
                    {
                        val = 0xa;
                        led_core_exp_04 |= 0x500;
                    }
                }
                else if (led_mux == (LED_SPEED_1G | LED_SPEED_100))
                {
                    val = 0x1;
                }
                else if (led_mux == (LED_SPEED_1G | LED_SPEED_10))
                {
                    val = 0x0;
                }
                else if (led_mux == LED_SPEED_1G)
                {
                    val = 0x3;
                }
                else if (led_mux == LED_SPEED_100)
                {
                    val = 0x1;
                }
                else if (led_mux == LED_SPEED_10)
                {
                    val = 0x0;
                }
                else if (led_mux == (LED_SPEED_100 | LED_SPEED_10))
                {
                    val = 0xa;
                    led_core_exp_04 |= 0x504;
                }
                else 
                {
                    val = 0xe;
                }
            }
            else
            {
                if (led_mux == LED_SPEED_ALL || led_mux == LED_SPEED_GBE)
                {
                    val = 0xa;
                    val2 = 0x2;
                }
                else if (led_activity == LED_SPEED_ALL || led_activity == LED_SPEED_GBE)
                {
                    val = 0xa;
                    val2 = 0x8;
                }
                else 
                {
                    val=0xe;
                    val2 = 0x0;
                }
                led_core_exp_04 |= (0x100 | val2<<(4*(j%2)));
            }


            led_sel = (val<<(4*((j+is_shifted)%2)));

            if (j < (2 - is_shifted))
                led_shd1c_0d |= led_sel;
            else
                led_shd1c_0e |= led_sel;
        }
    }

    ret = ret ? ret : phy_dev_write(phy_dev, RDB_ACCESS | CORE_SHD1C_0D, led_shd1c_0d);
    ret = ret ? ret : phy_dev_write(phy_dev, RDB_ACCESS | CORE_SHD1C_0E, led_shd1c_0e);
    if (led_shd1c_02 && !ret)
        ret = ret ? ret : phy_dev_write(phy_dev, RDB_ACCESS | CORE_SHD1C_02, led_shd1c_02);
    if (led_shd1c_09 && !ret)
        ret = phy_dev_write(phy_dev, RDB_ACCESS | CORE_SHD1C_09, led_shd1c_09);
    if (led_core_exp_04 && !ret)
        ret = phy_dev_write(phy_dev, RDB_ACCESS | CORE_EXP04, led_core_exp_04);

    printk("CORE_SHD1C_02: 0x%x CORE_SHD1C_09: 0x%x CORE_SHD1C_0D: 0x%x CORE_SHD1C_0E: 0x%x CORE_EXP04: 0x%x\n", 
        led_shd1c_02, led_shd1c_09, led_shd1c_0d, led_shd1c_0e, led_core_exp_04);

    return ret;
}

static int _phy_leds_init_xrdp(phy_dev_t *phy_dev, void *leds_info)
{
    return xrdp_leds_init(leds_info);
}

static int _phy_leds_init(phy_dev_t *phy_dev, void *leds_info)
{
    uint32_t phyid = 0;    

    mii_phyid_get(phy_dev, &phyid);

    printk("UNIMAC PHY ID 0x%x\n", phyid & PHY_ID_MASK);

    switch (phyid & PHY_ID_MASK)
    {
    case PHY_ID_5371_MASK:
    case PHY_ID_5321_MASK:
        return _phy_leds_init_xrdp(phy_dev, leds_info);
        break;
    case PHY_ID_51F1_MASK:
        return _phy_leds_init_51XX(phy_dev, leds_info, 1);
        break;
    default:
        return _phy_leds_init_51XX(phy_dev, leds_info, 0);
        break;
    }
}

phy_drv_t phy_drv_6846_egphy =
{
    .phy_type = PHY_TYPE_6846_EGPHY,
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
    .wirespeed_set = brcm_egphy_eth_wirespeed_set,
    .wirespeed_get = brcm_egphy_eth_wirespeed_get,
    .init = _phy_init,
    .dev_add = _phy_dev_add,
    .dev_del = _phy_dev_del,
    .drv_init = _phy_drv_init,
    .leds_init = _phy_leds_init,
    .cable_diag_run = brcm_cable_diag_run,
};
