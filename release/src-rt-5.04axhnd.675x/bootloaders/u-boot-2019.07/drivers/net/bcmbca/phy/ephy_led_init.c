// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2016 Broadcom Corporation
   All Rights Reserved

    
*/

/*
 *  Created on: Apr 2019
 *      Author: samyon.furman@broadcom.com
 */

#include <linux/ioport.h>
#include <linux/io.h>
#include "os_dep.h"
#include "bcm_bca_leds_dt_bindings.h"
#include "bcm_bca_leds.h"
#include "dt_access.h"

typedef union {
    struct {
        uint32_t rx_act_en                 :1 ; // [00:00]
        uint32_t tx_act_en                 :1 ; // [01:01]
        uint32_t spdlnk_led0_act_sel       :1 ; // [02:02]
        uint32_t spdlnk_led1_act_sel       :1 ; // [03:03]
        uint32_t spdlnk_led2_act_sel       :1 ; // [04:04]
        uint32_t act_led_act_sel           :1 ; // [05:05]
        uint32_t spdlnk_led0_act_pol_sel   :1 ; // [06:06]
        uint32_t spdlnk_led1_act_pol_sel   :1 ; // [07:07]
        uint32_t spdlnk_led2_act_pol_sel   :1 ; // [08:08]
        uint32_t act_led_pol_sel           :1 ; // [09:09]
        uint32_t led_spd_ovrd              :3 ; // [12:10]
        uint32_t lnk_status_ovrd           :1 ; // [13:13]
        uint32_t spd_ovrd_en               :1 ; // [14:14]
        uint32_t lnk_ovrd_en               :1 ; // [15:15]
        uint32_t r1                        :16; // [31:16]
    }Bits;
    uint32_t Reg32;
}EPHY_LED_CTRL;

typedef union {
    struct {
        uint32_t sel_no_link_encode        :3 ; // [02:00] 
        uint32_t sel_10m_encode            :3 ; // [05:03]
        uint32_t sel_100m_encode           :3 ; // [08:06]
        uint32_t sel_1000m_encode          :3 ; // [11:09]
        uint32_t sel_2500m_encode          :3 ; // [14:12]
        uint32_t sel_10g_encode            :3 ; // [17:15]
        uint32_t r1                        :14; // [31:18]
    }Bits;
    uint32_t Reg32;
}EPHY_LED_LINK_SPEED_ENC_SEL;

typedef union {
    struct {
        uint32_t no_link_encode            :3 ; // [02:00]
        uint32_t m10_encode                :3 ; // [05:03]
        uint32_t m100_encode               :3 ; // [08:06]
        uint32_t m1000_encode              :3 ; // [11:09]
        uint32_t m2500_encode              :3 ; // [14:12]
        uint32_t m10g_encode               :3 ; // [17:15]
        uint32_t r1                        :14; // [31:18]
    }Bits;
    uint32_t Reg32;
}EPHY_LED_LINK_SPEED_ENC;

typedef union {
    struct {
        uint32_t port_en                   :16; // [15:00]
        uint32_t act_sel                   :1 ; // [16:16]
        uint32_t act_pol_sel               :1 ; // [17:17]
        uint32_t lnk_pol_sel               :1 ; // [18:18]
        uint32_t r1                        :13; // [31:19]
    }Bits;
    uint32_t Reg32;
}EPHY_LED_AGGREGATE_CTRL;

typedef struct led_reg
{
    EPHY_LED_CTRL                   led_ctrl;
    EPHY_LED_LINK_SPEED_ENC_SEL     led_link_speed_enc_sel;
    EPHY_LED_LINK_SPEED_ENC         led_link_speed_enc;
}LED_REG;


static int driver_init_done=0;
struct EphyLedRegs {
    int                               max_led_regs;
    LED_REG                           **led_reg;
    uint32_t                          *led_blink_rate_ctrl;
    uint32_t                          *led_pwm_ctrl;
    uint32_t                          *led_intensity_ctrl;
    EPHY_LED_AGGREGATE_CTRL           *led_aggregate_ctrl;
    uint32_t                          *led_aggregate_blink_rate_ctrl;
    uint32_t                          *led_aggregate_pwm_ctrl;
    uint32_t                          *led_aggregate_intensity_ctrl;
    uint32_t                          *led_sw_init_ctrl;
} EphyLedRegs;

#define PEPHY_LED            ((volatile struct EphyLedRegs * const) &EphyLedRegs)


static int ephyled_probe(dt_device_t *pdev)
{
    int ret;
    struct resource res;
    int index=0;
    char led_reg_str[25];


    PEPHY_LED->max_led_regs=dt_property_read_u32(dt_dev_get_handle(pdev), "led_reg_max");
    // assume that there is only 1 led_reg if led_reg_max is not defined
    if(PEPHY_LED->max_led_regs == -1)
        PEPHY_LED->max_led_regs=1; 


    PEPHY_LED->led_reg = calloc(sizeof(LED_REG*), PEPHY_LED->max_led_regs);
    if(PEPHY_LED->led_reg == NULL)
    {
        dev_err(&pdev->dev, "memory allocation failed for led_reg\n");
        return -ENOMEM;
        
    }
    do
    {
        sprintf(led_reg_str, "led_reg_%d", index); 

        ret = dev_read_resource_byname(pdev, led_reg_str, &res);
        if (ret)
        {
            PEPHY_LED->led_reg[index]=NULL;
            continue;
        }
        PEPHY_LED->led_reg[index] = devm_ioremap(pdev, res.start, resource_size(&res));

    } while(++index < PEPHY_LED->max_led_regs);

    ret = dev_read_resource_byname(pdev, "aggregate_ctrl", &res);
    if (ret)
    {
        PEPHY_LED->led_aggregate_ctrl=NULL;
        dev_err(&pdev->dev, "Missing ephyled_base entry, ignoring\n");
    }
    else
        PEPHY_LED->led_aggregate_ctrl = devm_ioremap(pdev, res.start, resource_size(&res));

    dev_info(&pdev->dev, "registered\n");
    driver_init_done=1;

    return 0;
}

#ifdef __UBOOT__
static const struct udevice_id ephyled_ids[] = {
    { .compatible = "brcm,ephy-led" },
    { /* end of list */ },
};

U_BOOT_DRIVER(brcm_ephyled) = {
    .name	= "brcm-ephyled",
    .id	= UCLASS_MISC,
    .of_match = ephyled_ids,
    .probe = ephyled_probe,
};
#else
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
#endif


int ephy_leds_init(void *_leds_info)
{
    bca_leds_info_t *leds_info = (bca_leds_info_t *)_leds_info;
    int ret = 0;
    int j;
    EPHY_LED_CTRL led_ctrl;
    EPHY_LED_LINK_SPEED_ENC_SEL spdlnk_sel;
    EPHY_LED_LINK_SPEED_ENC spdlnk;
    uint32_t port = leds_info->port_id;
    EPHY_LED_AGGREGATE_CTRL agg_ctrl;

    uint8_t activity;


    if(driver_init_done == 0)
        return -1;
    if(port == 0xff || port >= PEPHY_LED->max_led_regs 
        || PEPHY_LED->led_reg == NULL ||  PEPHY_LED->led_reg[port] == NULL )
    {
        /*No Led data provided */
        return 0;
    }

    spdlnk_sel.Bits.sel_10g_encode = 0;
    spdlnk_sel.Bits.sel_1000m_encode = 0;
    spdlnk_sel.Bits.sel_100m_encode = 0;
    spdlnk_sel.Bits.sel_2500m_encode = 0;
    spdlnk_sel.Bits.sel_10m_encode = 0;
    spdlnk_sel.Bits.sel_no_link_encode = 0;

    spdlnk.Bits.m10g_encode = 7;
    spdlnk.Bits.m2500_encode = 7;
    spdlnk.Bits.m1000_encode = 7;
    spdlnk.Bits.m100_encode = 7;
    spdlnk.Bits.m10_encode = 7;
    spdlnk.Bits.no_link_encode = 7;

    led_ctrl.Reg32 = PEPHY_LED->led_reg[port]->led_ctrl.Reg32;


    if ( PEPHY_LED->led_aggregate_ctrl != NULL )
    {
        agg_ctrl.Reg32 = PEPHY_LED->led_aggregate_ctrl->Reg32; 
        agg_ctrl.Bits.act_sel = 0;
        agg_ctrl.Bits.act_pol_sel = 0;
        agg_ctrl.Bits.lnk_pol_sel = 0;
#if defined(CONFIG_BCM94908)
	/* link led polarity is reversed from hw.  Suppose to be active low but it is active high.
	 * change the polarity in led ctrl registr and also enable all 5 GPHY ports */
        agg_ctrl.Bits.lnk_pol_sel = 1;
#endif

        PEPHY_LED->led_aggregate_ctrl->Reg32 = agg_ctrl.Reg32;

        if (!leds_info->skip_in_aggregate)
            PEPHY_LED->led_aggregate_ctrl->Bits.port_en |= 1<<port;
    }

    for (j = 0; j < 3; j++)
    {
        uint32_t led_mux = leds_info->link[j];
        uint32_t led_activity = leds_info->activity[j];
        activity = 0;

        if (led_mux & LED_SPEED_10G)
            spdlnk.Bits.m10g_encode &= ~(1<<j);
        if (led_mux & LED_SPEED_2500)
            spdlnk.Bits.m2500_encode &= ~(1<<j);
        if (led_mux & LED_SPEED_1G)
            spdlnk.Bits.m1000_encode &= ~(1<<j);
        if (led_mux & LED_SPEED_100)
            spdlnk.Bits.m100_encode &= ~(1<<j);
        if (led_mux & LED_SPEED_10)
            spdlnk.Bits.m10_encode &= ~(1<<j);

        if (led_activity & LED_SPEED_10G)
            spdlnk_sel.Bits.sel_10g_encode |= (1<<j);
        if (led_activity & LED_SPEED_2500)
            spdlnk_sel.Bits.sel_2500m_encode |= (1<<j);
        if (led_activity & LED_SPEED_1G)
            spdlnk_sel.Bits.sel_1000m_encode |= (1<<j);
        if (led_activity & LED_SPEED_100)
            spdlnk_sel.Bits.sel_100m_encode |= (1<<j);
        if (led_activity & LED_SPEED_10)
            spdlnk_sel.Bits.sel_10m_encode |= (1<<j);

        /* to configure the speed led to show activity only for specified
           speeds */
        if (led_activity && !(led_mux & led_activity))
            activity = 1;

        switch (j)
        {
        case 0:
            led_ctrl.Bits.spdlnk_led0_act_sel = activity;
            break;
        case 1:
            led_ctrl.Bits.spdlnk_led1_act_sel = activity;
            break;
        case 2:
            led_ctrl.Bits.spdlnk_led2_act_sel = activity;
            break;
        }
    }
            
    if (leds_info->activity[3] || leds_info->link[3])
        led_ctrl.Bits.act_led_act_sel = leds_info->link[3] ? 0 : 1;

    PEPHY_LED->led_reg[port]->led_link_speed_enc_sel.Reg32 = spdlnk_sel.Reg32;
    PEPHY_LED->led_reg[port]->led_link_speed_enc.Reg32 = spdlnk.Reg32;
    PEPHY_LED->led_reg[port]->led_ctrl.Reg32 = led_ctrl.Reg32;
    printk("%s Port %d configured \n", __FUNCTION__, port);

    return ret;
}
