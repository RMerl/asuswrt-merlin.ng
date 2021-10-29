/*
   Copyright (c) 2016 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2016:DUAL/GPL:standard
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License, version 2, as published by
    the Free Software Foundation (the "GPL").
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    
    A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
    writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
    
:>
*/

/*
 *  Created on: Apr 2019
 *      Author: samyon.furman@broadcom.com
 */
#include "os_dep.h"
#include "bcm_map_part.h"
#include "boardparms.h"

int xrdp_led_init(int port)
{
    int ret = 0;
#if defined(_BCM96878_) || defined (CONFIG_BCM96878)
    int j;
    XRDP_LED_CTRL led_ctrl;
    XRDP_LED_LINK_SPEED_ENC_SEL spdlnk_sel;
    XRDP_LED_LINK_SPEED_ENC spdlnk;
    LEDS_ADVANCED_INFO led_info = {};
    uint8_t activity;

    ret = BpGetLedsAdvancedInfo(&led_info);
    if (ret != BP_SUCCESS )
    {
        printk("Error reading Led Advanced info from board params\n");
        goto Exit;
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

    XRDP_LED->led_aggregate_ctrl.Bits.act_sel = 1;
    XRDP_LED->led_aggregate_ctrl.Bits.act_pol_sel = 1;

    led_ctrl.Reg32 = XRDP_LED->led_ctrl[port].Reg32;
    if (!led_info.ledInfo[port].skip_in_aggregate)
        XRDP_LED->led_aggregate_ctrl.Bits.port_en |= 1<<port;

    for (j = 0; j < (MAX_LEDS_PER_PORT - 1); j++)
    {
        uint32_t led_mux = led_info.ledInfo[port].SpeedLed[j] & BP_NET_LED_SPEED_MASK;
        uint32_t led_activity = led_info.ledInfo[port].ActivityLed[j] & BP_NET_LED_SPEED_MASK;
        activity = 0;

        if (led_mux & BP_NET_LED_SPEED_10G)
            spdlnk.Bits.m10g_encode &= ~(1<<j);
        if (led_mux & BP_NET_LED_SPEED_2500)
            spdlnk.Bits.m2500_encode &= ~(1<<j);
        if (led_mux & BP_NET_LED_SPEED_1G)
            spdlnk.Bits.m1000_encode &= ~(1<<j);
        if (led_mux & BP_NET_LED_SPEED_100)
            spdlnk.Bits.m100_encode &= ~(1<<j);
        if (led_mux & BP_NET_LED_SPEED_10)
            spdlnk.Bits.m10_encode &= ~(1<<j);

        if (led_activity & BP_NET_LED_SPEED_10G)
            spdlnk_sel.Bits.sel_10g_encode |= (1<<j);
        if (led_activity & BP_NET_LED_SPEED_2500)
            spdlnk_sel.Bits.sel_2500m_encode |= (1<<j);
        if (led_activity & BP_NET_LED_SPEED_1G)
            spdlnk_sel.Bits.sel_1000m_encode |= (1<<j);
        if (led_activity & BP_NET_LED_SPEED_100)
            spdlnk_sel.Bits.sel_100m_encode |= (1<<j);
        if (led_activity & BP_NET_LED_SPEED_10)
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
            
    if (led_info.is_activity_led_present)
    {
        if (led_info.ledInfo[port].SpeedLed[3] & BP_NET_LED_SPEED_MASK)
                    led_ctrl.Bits.act_led_act_sel = 0;
        else
            led_ctrl.Bits.act_led_act_sel = 1;
    }

    XRDP_LED->led_link_speed_enc_sel[port].Reg32 = spdlnk_sel.Reg32;
    XRDP_LED->led_link_speed_enc[port].Reg32 = spdlnk.Reg32;
    XRDP_LED->led_ctrl[port].Reg32 = led_ctrl.Reg32;
Exit:
#endif
    return ret;

}
