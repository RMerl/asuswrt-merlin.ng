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

#include "os_dep.h"
#include "bcm_bca_leds_dt_bindings.h"
#include "bcm_bca_leds.h"
#include "dt_access.h"

static uintptr_t xrdpled_base;

static int xrdpled_probe(dt_device_t *pdev)
{
    int ret;

    xrdpled_base = dt_dev_remap_resource(pdev, 0);
    if (IS_ERR(xrdpled_base))
    {
        ret = PTR_ERR(xrdpled_base);
        xrdpled_base = NULL;
        dev_err(&pdev->dev, "Missing xrdpled_base entry\n");
        goto Exit;
    }

    dev_dbg(&pdev->dev, "xrdpled_base=0x%lx\n", xrdpled_base);
    dev_info(&pdev->dev, "registered\n");

    return 0;

Exit:
    return ret;
}

static const struct of_device_id of_platform_table[] = {
    { .compatible = "brcm,xrdp-led" },
    { /* end of list */ },
};

static struct platform_driver of_platform_driver = {
    .driver = {
        .name = "brcm-xrdpled",
        .of_match_table = of_platform_table,
    },
    .probe = xrdpled_probe,
};
module_platform_driver(of_platform_driver);

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
}XRDP_LED_CTRL;

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
}XRDP_LED_LINK_SPEED_ENC_SEL;

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
}XRDP_LED_LINK_SPEED_ENC;

typedef union {
    struct {
        uint32_t port_en                   :16; // [15:00]
        uint32_t act_sel                   :1 ; // [16:16]
        uint32_t act_pol_sel               :1 ; // [17:17]
        uint32_t lnk_pol_sel               :1 ; // [18:18]
        uint32_t r1                        :13; // [31:19]
    }Bits;
    uint32_t Reg32;
}XRDP_LED_AGGREGATE_CTRL;

typedef struct XrdpLedRegs {
    XRDP_LED_CTRL                   led_ctrl[5];
    uint32_t                          reserved0[3];
    XRDP_LED_LINK_SPEED_ENC_SEL     led_link_speed_enc_sel[5];
    uint32_t                          reserved1[3];
    XRDP_LED_LINK_SPEED_ENC         led_link_speed_enc[5];
    uint32_t                          reserved2[3];
    uint32_t                          led_blink_rate_ctrl;
    uint32_t                          led_pwm_ctrl;
    uint32_t                          led_intensity_ctrl;
    XRDP_LED_AGGREGATE_CTRL         led_aggregate_ctrl;
    uint32_t                          led_aggregate_blink_rate_ctrl;
    uint32_t                          led_aggregate_pwm_ctrl;
    uint32_t                          led_aggregate_intensity_ctrl;
    uint32_t                          led_sw_init_ctrl;
} XrdpLedRegs;

#define XRDP_LED_BASE       (void *)xrdpled_base
#define XRDP_LED_REGS       ((volatile XrdpLedRegs * const) XRDP_LED_BASE)

int xrdp_leds_init(void *_leds_info)
{
    bca_leds_info_t *leds_info = (bca_leds_info_t *)_leds_info;
    int ret = 0;
    int j;
    XRDP_LED_CTRL led_ctrl;
    XRDP_LED_LINK_SPEED_ENC_SEL spdlnk_sel;
    XRDP_LED_LINK_SPEED_ENC spdlnk;
    uint32_t port = leds_info->port_id;
    XRDP_LED_AGGREGATE_CTRL agg_ctrl;

    uint8_t activity;

    if(port == 0xff)
    {
        /* No Led info provided */
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

    led_ctrl.Reg32 = XRDP_LED_REGS->led_ctrl[port].Reg32;

    agg_ctrl.Reg32 = XRDP_LED_REGS->led_aggregate_ctrl.Reg32;
    agg_ctrl.Bits.act_sel = 1;
    agg_ctrl.Bits.act_pol_sel = 0;
    agg_ctrl.Bits.lnk_pol_sel = 1;

    if (!leds_info->skip_in_aggregate)
        agg_ctrl.Bits.port_en |= 1<<port;
    XRDP_LED_REGS->led_aggregate_ctrl.Reg32 = agg_ctrl.Reg32;

    if (!leds_info->skip_in_aggregate)
        XRDP_LED_REGS->led_aggregate_ctrl.Bits.port_en |= 1<<port;

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

    XRDP_LED_REGS->led_link_speed_enc_sel[port].Reg32 = spdlnk_sel.Reg32;
    XRDP_LED_REGS->led_link_speed_enc[port].Reg32 = spdlnk.Reg32;
    XRDP_LED_REGS->led_ctrl[port].Reg32 = led_ctrl.Reg32;
    printk("%s Port %d configured \n", __FUNCTION__, port);

    return ret;
}
