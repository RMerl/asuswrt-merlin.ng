/*
   Copyright (c) 2016 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2021:DUAL/GPL:standard

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

#include <linux/slab.h>

#include "os_dep.h"
#include "bcm_bca_leds_dt_bindings.h"
#include "bcm_bca_leds.h"
#include "dt_access.h"

#define MAX_LED_SW_NUM             2
#define MAX_LED_PORT_NUM           8

typedef union {
    struct {
        uint32_t no_link_encode            :2 ; // [01:00]
        uint32_t m10_encode                :2 ; // [03:02]
        uint32_t m100_encode               :2 ; // [05:04]
        uint32_t m1000_encode              :2 ; // [07:06]
        uint32_t sel_no_link_encode        :2 ; // [09:08] 
        uint32_t sel_10m_encode            :2 ; // [11:10]
        uint32_t sel_100m_encode           :2 ; // [13:12]
        uint32_t sel_1000m_encode          :2 ; // [15:14]	  
        uint32_t rx_dv_en                  :1 ; // [16:16]
        uint32_t tx_en_en                  :1 ; // [17:17]
        uint32_t spdlnk_led0_act_sel       :2 ; // [19:18]
        uint32_t spdlnk_led1_act_sel       :2 ; // [21:20]
        uint32_t act_led_act_sel           :2 ; // [23:22]
        uint32_t spdlnk_src_sel            :1 ; // [24:24]	  
        uint32_t spdlnk_led0_act_pol_sel   :1 ; // [25:25]
        uint32_t spdlnk_led1_act_pol_sel   :1 ; // [26:26]
        uint32_t act_led_pol_sel           :1 ; // [27:27]
        uint32_t r1                        :4 ; // [31:28]
    }Bits;
    uint32_t Reg32;
}EPHY_LED_CTRL;

#define LED_REG  EPHY_LED_CTRL

static int driver_init_done=0;
struct EphyLedRegs {
    int                               max_led_regs;
    LED_REG                           **led_reg;
} EphyLedRegs;

#define PEPHY_LED            ((volatile struct EphyLedRegs * const) &EphyLedRegs)

/*
 * mapping for the led reg idx based on led port info(sw:port). See ephyled node 
 * in the chip.dtsi for led idx. Each led register used has the name of led_reg_<idx>.
 */
int led_port2idx_map[MAX_LED_SW_NUM][MAX_LED_PORT_NUM] = {
    /* sw 0 rnr: only port 0 connects to crossbar as wan port*/
    {  5, -1, -1, -1,  -1,  -1, -1, -1 },
    /* sw 1 sf2: port 0-4 map to sw led reg */
    {  0,  1,  2,  3,  4, -1, -1, -1 }
};

static int ephyled_probe(dt_device_t *pdev)
{
    struct resource *res;
    int index=0;
    char led_reg_str[25];

    // assume that there is only 1 led_reg if led_reg_max is not defined
    PEPHY_LED->max_led_regs=1;
    of_property_read_u32(pdev->dev.of_node, "led_reg_max", (uint32_t*)&PEPHY_LED->max_led_regs);

    PEPHY_LED->led_reg = kzalloc(PEPHY_LED->max_led_regs*sizeof(LED_REG*), GFP_KERNEL);
    if(PEPHY_LED->led_reg == NULL)
    {
        dev_err(&pdev->dev, "memory allocation failed for led_reg\n");
        return -ENOMEM;
    }
    do
    {
        sprintf(led_reg_str, "led_reg_%d", index);
        res = platform_get_resource_byname(pdev, IORESOURCE_MEM, led_reg_str);
        //some may have missing led register for certain led
        if (res == NULL || IS_ERR((void*)res->start))
            continue;
        PEPHY_LED->led_reg[index] = devm_ioremap_resource(&pdev->dev, res);
    } while(++index < PEPHY_LED->max_led_regs);

    dev_info(&pdev->dev, "registered\n");

    driver_init_done=1;

    return 0;
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
    int ret = 0, led_idx;
    int j;
    EPHY_LED_CTRL led_ctrl;
    uint32_t port;
    uint32_t sw;
    uint8_t activity;

    if (driver_init_done == 0)
       return -1;
	
    if (leds_info == NULL)
       return -1;

    port = leds_info->port_id;
    sw = leds_info->sw_id;
    if (port >= MAX_LED_PORT_NUM || sw >= MAX_LED_SW_NUM) {
        /* No Led data provided */
        printk("invalid port %d or sw %d number!\n", port, sw);
        return 0;
    }

    led_idx = led_port2idx_map[sw][port];
    if (led_idx == -1 || led_idx >= PEPHY_LED->max_led_regs) {
        printk("invalid led idx %d max led reg %d!\n", led_idx, PEPHY_LED->max_led_regs);
        return 0;
    }
    if (PEPHY_LED->led_reg[led_idx] == NULL) {
        printk("led reg %d not defined or mapped!\n", led_idx);
        return 0;
    }

    led_ctrl.Reg32 = PEPHY_LED->led_reg[led_idx]->Reg32;

    led_ctrl.Bits.sel_1000m_encode = 0;
    led_ctrl.Bits.sel_100m_encode = 0;
    led_ctrl.Bits.sel_10m_encode = 0;
    led_ctrl.Bits.sel_no_link_encode = 0;

    led_ctrl.Bits.m1000_encode = 3;
    led_ctrl.Bits.m100_encode = 3;
    led_ctrl.Bits.m10_encode = 3;
    led_ctrl.Bits.no_link_encode = 3;

    for (j = 0; j < 2; j++)
    {
        uint32_t led_mux = leds_info->link[j];
        uint32_t led_activity = leds_info->activity[j];
        activity = 0;

        if (led_mux & LED_SPEED_1G)
            led_ctrl.Bits.m1000_encode &= ~(1<<j);
        if (led_mux & LED_SPEED_100)
            led_ctrl.Bits.m100_encode &= ~(1<<j);
        if (led_mux & LED_SPEED_10)
            led_ctrl.Bits.m10_encode &= ~(1<<j);

        if (led_activity & LED_SPEED_1G)
            led_ctrl.Bits.sel_1000m_encode |= (1<<j);
        if (led_activity & LED_SPEED_100)
            led_ctrl.Bits.sel_100m_encode |= (1<<j);
        if (led_activity & LED_SPEED_10)
            led_ctrl.Bits.sel_10m_encode |= (1<<j);

        /* to configure the speed led to show activity only for specified
           speeds */
        if (led_activity && !(led_mux & led_activity))
            activity = 2;

        switch (j)
        {
        case 0:
            led_ctrl.Bits.spdlnk_led0_act_sel = activity;
            break;
        case 1:
            led_ctrl.Bits.spdlnk_led1_act_sel = activity;
            break;
        }
    }

    if (leds_info->activity[2] || leds_info->link[2])
        led_ctrl.Bits.act_led_act_sel = leds_info->link[2] ? 0 : 2;


    PEPHY_LED->led_reg[led_idx]->Reg32 = led_ctrl.Reg32;
    printk("%s SW %d Port %d led idx %d configured to 0x%08x\n",
        __FUNCTION__, sw, port, led_idx, led_ctrl.Reg32);

    return ret;
}
