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
 *  Created on: Apr 2019
 *      Author: samyon.furman@broadcom.com
 */

#include <linux/slab.h>

#include "os_dep.h"
#include "bcm_bca_leds_dt_bindings.h"
#include "bcm_bca_leds.h"
#include "dt_access.h"

typedef struct EPHY_LED_CTRL {
        uint32_t no_link_encode              :2 ; // [00:01]
        uint32_t m10_encode                  :2 ; // [02:03]
        uint32_t m100_encode                 :2 ; // [04:05]
        uint32_t m1000_encode                :2 ; // [06:07]
        uint32_t sel_no_link_encode          :2 ; // [08:09]
        uint32_t sel_10m_encode              :2 ; // [10:11]
        uint32_t sel_100m_encode             :2 ; // [12:13]
        uint32_t sel_1000m_encode            :2 ; // [14:15]
        uint32_t rx_dv_en                    :1 ; // [16:16];
        uint32_t tx_en_en                    :1 ; // [17:17];
        uint32_t spdlnk_led0_act_sel         :2 ; // [18:19]                     
        uint32_t spdlnk_led1_act_sel         :2 ; // [20:21] 
        uint32_t act_led_act_sel             :2 ; // [22:23] 
        uint32_t spdlnk_src_sel              :1 ; // [24:24] 
        uint32_t spdlnk_led0_act_pol_sel     :1 ; // [25:25]
        uint32_t spdlnk_led1_act_pol_sel     :1 ; // [26:26]
        uint32_t act_led_pol_sel             :1 ; // [27:27]
        uint32_t reserved                    :4 ; // [28:31]
}EPHY_LED_CTRL ;

typedef struct led_reg {
    union {
        struct EPHY_LED_CTRL led_ctrl;
        uint32_t Reg32;

    };
}LED_REG;

static int driver_init_done=0;
struct EphyLedRegs {
    LED_REG                           *led_reg;
} EphyLedRegs;

#define PEPHY_LED            ((volatile struct EphyLedRegs * const) &EphyLedRegs)


static int ephyled_probe(dt_device_t *pdev)
{
    struct resource *res;
    int ret=-1;
    
    res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "led_reg");
    //some may have missing led register for certain led
    if (res != NULL && !IS_ERR(res->start))
    {
        PEPHY_LED->led_reg = devm_ioremap_resource(&pdev->dev, res);
        dev_info(&pdev->dev, "registered\n");
        driver_init_done=1;
        ret=0;
    }

    return ret;
}

static const struct of_device_id of_platform_table[] = {
    { .compatible = "brcm,ephy-led" },
    { /* end of list */ },
};

static struct platform_driver of_platform_driver = {
    .driver = {
        .name = "brcm-ephyled",
        .of_match_table = of_platform_table,
    },
    .probe = ephyled_probe,
};
module_platform_driver(of_platform_driver);

int ephy_leds_init(void *_leds_info)
{
    bca_leds_info_t *leds_info = (bca_leds_info_t *)_leds_info;
    int ret = 0;
    int j;
    LED_REG led_reg;
    uint32_t port = leds_info->port_id;

    if(driver_init_done == 0) 
        return -1;
    if(port == 0xff || port >= 5)
    {
        /* No Led data provided */
        return 0;
    }


    led_reg.Reg32 = PEPHY_LED->led_reg[port].Reg32;

    led_reg.led_ctrl.m1000_encode = 3;
    led_reg.led_ctrl.m100_encode = 3;
    led_reg.led_ctrl.m10_encode = 3;
    led_reg.led_ctrl.no_link_encode = 3;



    for (j = 0; j < 2; j++)
    {
        uint32_t led_mux = leds_info->link[j];

        if (led_mux & LED_SPEED_1G)
            led_reg.led_ctrl.m1000_encode &= ~(1<<j);
        if (led_mux & LED_SPEED_100)
            led_reg.led_ctrl.m100_encode &= ~(1<<j);
        if (led_mux & LED_SPEED_10)
            led_reg.led_ctrl.m10_encode &= ~(1<<j);
    }

    PEPHY_LED->led_reg[port].Reg32 = led_reg.Reg32;
    printk("%s Port %d configured\n", __FUNCTION__, port);

    return ret;
}
