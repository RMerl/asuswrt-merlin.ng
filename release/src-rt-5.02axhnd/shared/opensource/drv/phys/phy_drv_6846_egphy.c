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
#include "bcm_map_part.h"
#include "boardparms.h"

#define QEGPHY_CTRL_REG     QEGPHY_BASE + 0x0008
#define CORE_SHD1C_09			0x0019 /* LED Control */
#define CORE_SHD1C_0D			0x001d /* LED Selector 1 */
#define CORE_SHD1C_0E			0x001e /* LED Selector 2 */
#define CORE_EXP04              0x0034 /* Bicollor Led Selector */

static uint32_t enabled_ports;

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

#define MAX_LEDS_PER_PORT_51F1 2
#define PHY_ID_51F1_MASK 0x51F1

/* The LEDS in 51F1 revision of the PHY are shifted LED0 -> LED1, LED1->LED2 */
static int _phy_leds_init_51F1(phy_dev_t *phy_dev)
{
    int ret = 0;
    int j;
    uint16_t led_shd1c_09 = 0;
    uint16_t led_core_exp_04 = 0;
    uint16_t led_shd1c_0d = 0, led_shd1c_0e = 0;
    LEDS_ADVANCED_INFO led_info = {};

    ret = BpGetLedsAdvancedInfo(&led_info);
    if (ret != BP_SUCCESS )
    {
        printk("Error reading Led Advanced info from board params\n");
        goto Exit;
    }

    for (j = 0; j < MAX_LEDS_PER_PORT; j++)
    {
        uint32_t led_mux = led_info.ledInfo[phy_dev->addr-1].SpeedLed[j] & BP_NET_LED_SPEED_MASK;
        uint32_t led_activity = led_info.ledInfo[phy_dev->addr-1].ActivityLed[j] & BP_NET_LED_SPEED_MASK;
        uint32_t val = 0;

        if (led_mux == led_activity)
        {
            if (led_mux == BP_NET_LED_SPEED_ALL || led_mux == BP_NET_LED_SPEED_GBE)
            {
                val = 0x18;
            }
            else if (led_mux == (BP_NET_LED_SPEED_1G | BP_NET_LED_SPEED_100))
            {
                val = 0x108;
            }
            else if (led_mux == (BP_NET_LED_SPEED_1G | BP_NET_LED_SPEED_10))
            {
                val = 0x108;
            }
            else if (led_mux == BP_NET_LED_SPEED_1G)
            {
                val = 0x118;
            }
            else if (led_mux == BP_NET_LED_SPEED_100)
            {
                val = 0x118;
            }
            else if (led_mux == BP_NET_LED_SPEED_10)
            {
                val = 0x118;
            }
            else if (led_mux == (BP_NET_LED_SPEED_100 | BP_NET_LED_SPEED_10))
            {
                val = 0x118;
            }

            if( val > led_shd1c_09)
                led_shd1c_09 = val;
        }
    }

    for (j = 0; j < MAX_LEDS_PER_PORT_51F1; j++)
    {
        uint16_t led_sel = 0;
        uint32_t led_mux = led_info.ledInfo[phy_dev->addr-1].SpeedLed[j] & BP_NET_LED_SPEED_MASK;
        uint32_t led_activity = led_info.ledInfo[phy_dev->addr-1].ActivityLed[j] & BP_NET_LED_SPEED_MASK;
        uint16_t val, val2;

        if (led_mux == led_activity)
        {
            if (led_mux == BP_NET_LED_SPEED_ALL || led_mux == BP_NET_LED_SPEED_GBE)
            {
                val = 0x3;
                if (led_shd1c_09 == 0x118)
                {
                    val = 0xa;
                    led_core_exp_04 |= 0x500;
                }
            }
            else if (led_mux == (BP_NET_LED_SPEED_1G | BP_NET_LED_SPEED_100))
            {
                val = 0x1;
            }
            else if (led_mux == (BP_NET_LED_SPEED_1G | BP_NET_LED_SPEED_10))
            {
                val = 0x0;
            }
            else if (led_mux == BP_NET_LED_SPEED_1G)
            {
                val = 0x3;
            }
            else if (led_mux == BP_NET_LED_SPEED_100)
            {
                val = 0x1;
            }
            else if (led_mux == BP_NET_LED_SPEED_10)
            {
                val = 0x0;
            }
            else if (led_mux == (BP_NET_LED_SPEED_100 | BP_NET_LED_SPEED_10))
            {
                val = 0xa;
                led_core_exp_04 |= (0x504);
            }
            else 
            {
                val = 0xe;
            }
        }
        else
        {
            if (led_mux == BP_NET_LED_SPEED_ALL || led_mux == BP_NET_LED_SPEED_GBE)
            {
                val = 0xa;
                val2 = 0x8;
                led_core_exp_04 |= (0x100 | val2<<(4*(j%2)));
            }
            else if (led_activity == BP_NET_LED_ACTIVITY_ALL || led_activity == BP_NET_LED_SPEED_GBE)
            {
                val = 0xa;
                val2 = 0x2;
            }
            else 
            {
                val=0xe;
                val2 = 0x0;
            }
            led_core_exp_04 |= (0x100 | val2<<(4*(j%2)));
        }
        
        led_sel = (val<<(4*((j+1)%2)));

        if (j < 1)
            led_shd1c_0d |= led_sel;
        else
            led_shd1c_0e |= led_sel;
    }
    
    ret = ret ? ret : phy_dev_write(phy_dev, RDB_ACCESS | CORE_SHD1C_0D, led_shd1c_0d);
    ret = ret ? ret : phy_dev_write(phy_dev, RDB_ACCESS | CORE_SHD1C_0E, led_shd1c_0e);
    if (led_shd1c_09 && !ret)
        ret = phy_dev_write(phy_dev, RDB_ACCESS | CORE_SHD1C_09, led_shd1c_09);
    if (led_core_exp_04 && !ret)
        ret = phy_dev_write(phy_dev, RDB_ACCESS | CORE_EXP04, led_core_exp_04);

    printk("CORE_SHD1C_09: 0x%x CORE_SHD1C_0D: 0x%x CORE_SHD1C_0E: 0x%x CORE_EXP04: 0x%x\n", 
        led_shd1c_09, led_shd1c_0d, led_shd1c_0e, led_core_exp_04);

Exit:
    return ret;
}

static int _phy_leds_init_51E1(phy_dev_t *phy_dev)
{
    int ret = 0;
    int j;
    uint16_t led_shd1c_09 = 0;
    uint16_t led_core_exp_04 = 0;
    uint16_t led_shd1c_0d = 0, led_shd1c_0e = 0;
    LEDS_ADVANCED_INFO led_info = {};

    ret = BpGetLedsAdvancedInfo(&led_info);
    if (ret != BP_SUCCESS )
    {
        printk("Error reading Led Advanced info from board params\n");
        goto Exit;
    }

    for (j = 0; j < MAX_LEDS_PER_PORT; j++)
    {
        uint32_t led_mux = led_info.ledInfo[phy_dev->addr-1].SpeedLed[j] & BP_NET_LED_SPEED_MASK;
        uint32_t led_activity = led_info.ledInfo[phy_dev->addr-1].ActivityLed[j] & BP_NET_LED_SPEED_MASK;
        uint32_t val = 0;

        if (led_mux == led_activity)
        {
            if (led_mux == BP_NET_LED_SPEED_ALL || led_mux == BP_NET_LED_SPEED_GBE)
            {
                val = 0x18;
            }
            else if (led_mux == (BP_NET_LED_SPEED_1G | BP_NET_LED_SPEED_100))
            {
                val = 0x108;
            }
            else if (led_mux == (BP_NET_LED_SPEED_1G | BP_NET_LED_SPEED_10))
            {
                val = 0x108;
            }
            else if (led_mux == BP_NET_LED_SPEED_1G)
            {
                val = 0x118;
            }
            else if (led_mux == BP_NET_LED_SPEED_100)
            {
                val = 0x118;
            }
            else if (led_mux == BP_NET_LED_SPEED_10)
            {
                val = 0x118;
            }
            else if (led_mux == (BP_NET_LED_SPEED_100 | BP_NET_LED_SPEED_10))
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
        uint32_t led_mux = led_info.ledInfo[phy_dev->addr-1].SpeedLed[j] & BP_NET_LED_SPEED_MASK;
        uint32_t led_activity = led_info.ledInfo[phy_dev->addr-1].ActivityLed[j] & BP_NET_LED_SPEED_MASK;
        uint16_t val, val2;

        if (led_mux == led_activity)
        {
            if (led_mux == BP_NET_LED_SPEED_ALL || led_mux == BP_NET_LED_SPEED_GBE)
            {
                val = 0x3;
                if (led_shd1c_09 == 0x118)
                {
                    val = 0xa;
                    led_core_exp_04 |= 0x500;
                }
            }
            else if (led_mux == (BP_NET_LED_SPEED_1G | BP_NET_LED_SPEED_100))
            {
                val = 0x1;
            }
            else if (led_mux == (BP_NET_LED_SPEED_1G | BP_NET_LED_SPEED_10))
            {
                val = 0x0;
            }
            else if (led_mux == BP_NET_LED_SPEED_1G)
            {
                val = 0x3;
            }
            else if (led_mux == BP_NET_LED_SPEED_100)
            {
                val = 0x1;
            }
            else if (led_mux == BP_NET_LED_SPEED_10)
            {
                val = 0x0;
            }
            else if (led_mux == (BP_NET_LED_SPEED_100 | BP_NET_LED_SPEED_10))
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
            if (led_mux == BP_NET_LED_SPEED_ALL || led_mux == BP_NET_LED_SPEED_GBE)
            {
                val = 0xa;
                val2 = 0x8;
                led_core_exp_04 |= (0x100 | val2<<(4*(j%2)));
            }
            else if (led_activity == BP_NET_LED_ACTIVITY_ALL || led_activity == BP_NET_LED_SPEED_GBE)
            {
                val = 0xa;
                val2 = 0x2;
            }
            else 
            {
                val=0xe;
                val2 = 0x0;
            }
            led_core_exp_04 |= (0x100 | val2<<(4*(j%2)));
        }
 
        
        led_sel = (val<<(4*(j%2)));

        if (j < 2)
            led_shd1c_0d |= led_sel;
        else
            led_shd1c_0e |= led_sel;
    }

    ret = ret ? ret : phy_dev_write(phy_dev, RDB_ACCESS | CORE_SHD1C_0D, led_shd1c_0d);
    ret = ret ? ret : phy_dev_write(phy_dev, RDB_ACCESS | CORE_SHD1C_0E, led_shd1c_0e);
    if (led_shd1c_09 && !ret)
        ret = phy_dev_write(phy_dev, RDB_ACCESS | CORE_SHD1C_09, led_shd1c_09);
    if (led_core_exp_04 && !ret)
        ret = phy_dev_write(phy_dev, RDB_ACCESS | CORE_EXP04, led_core_exp_04);

    printk("CORE_SHD1C_09: 0x%x CORE_SHD1C_0D: 0x%x CORE_SHD1C_0E: 0x%x CORE_EXP04: 0x%x\n", 
        led_shd1c_09, led_shd1c_0d, led_shd1c_0e, led_core_exp_04);

Exit:
    return ret;
}

static int _phy_leds_init(phy_dev_t *phy_dev)
{
    uint32_t phyid = 0;

    mii_phyid_get(phy_dev, &phyid);

    if ((phyid & PHY_ID_51F1_MASK) == PHY_ID_51F1_MASK)
        return _phy_leds_init_51F1(phy_dev);
    else
        return _phy_leds_init_51E1(phy_dev);
}

#define CORE_SHD18_000          0x0028 /* Auxiliary Control Register */
#define CORE_EXPB0              0x00b0 /* Bias Control 0 */
#define DSP_TAP10               0x0125 /* PLL Bandwidth Control */
#define DSP_TAP33_C2            0x0152 /* EEE LPI Timers */
#define DSP_TAP34_C2            0x0156 /* EEE 100TX Mode BW control */
#define DSP_FFE_ZERO_DET_CTL    0x0166 /* FFE Zero Detection Control */
#define AFE_TX_IQ_RX_LP         0x01e4 /* AFE_TX_IQ_RX_LP */
#define AFE_TX_CONFIG_0         0x01e5 /* AFE_TX_CONFIG_0 */
#define AFE_TX_CONFIG_1         0x01ea /* AFE_TX_CONFIG_1 */
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

    /* LPF BW */
    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | AFE_TX_IQ_RX_LP, 0x233f)))
        goto Exit;

    /* RCAL +6LSB to make impedance from 112 to 100ohm */
    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | AFE_TEMPSEN_OTHERS, 0xad40)))
        goto Exit;

    /* since rcal make R smaller, make master current -4% */
    if ((ret = phy_dev_write(phy_dev, RDB_ACCESS | DSP_TAP10, 0x091b)))
        goto Exit;

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
    
    if ((ret = _phy_leds_init(phy_dev)))
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

static int _phy_dev_add(phy_dev_t *phy_dev)
{
    uint32_t port = (unsigned long)phy_dev->priv;

    enabled_ports |= (1 << port);

    return 0;
}

static int _phy_dev_del(phy_dev_t *phy_dev)
{
    uint32_t port = (unsigned long)phy_dev->priv;

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
    .init = _phy_init,
    .dev_add = _phy_dev_add,
    .dev_del = _phy_dev_del,
    .drv_init = _phy_drv_init,
};
