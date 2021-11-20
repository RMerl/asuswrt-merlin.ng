/*
 * <:copyright-BRCM:2018:DUAL/GPL:standard
 * 
 *    Copyright (c) 2018 Broadcom 
 *    All Rights Reserved
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 * 
 * :>
 */
// BCMFORMAT: notabs reindent:uncrustify:bcm_minimal_i4.cfg

#include "boardparms.h"
#include "shared_utils.h"

#ifdef _CFE_                                                
#include "lib_types.h"
#include "lib_printf.h"
#include "lib_string.h"
#include "bcm_map.h"
#define printk  printf
#else // Linux
#include <linux/kernel.h>
#include <linux/module.h>
#include <bcm_map_part.h>
#include <linux/string.h>
#endif

//only enable it for CFE
//#define DEBUG_PHYS_INTF

void bcm_ethsw_led_init(void)
{
    PHYS_INTF_ADV_LEDS_INFO led_info[BP_MAX_PHYS_INTF_PORTS];
    int port, cnt=BP_MAX_PHYS_INTF_PORTS, index;
    int rc;
    uint16_t lnkAggrLed, actAggrLed;
    volatile LED_CFG *led_cfg=NULL;
    uint32 sel_mask, encode_mask, sel_value,encode_value;
    uint32 m10g_encode;
    uint32 m2500_encode;
    uint32 m1000_encode;
    uint32 m100_encode;
    uint32 m10_encode;
    uint32 sel_10g_encode;
    uint32 sel_2500m_encode;
    uint32 sel_1000m_encode;
    uint32 sel_100m_encode;
    uint32 sel_10m_encode;
    uint32 act_tx_rx_mask;
    uint32 act_tx_rx_value=0;
    uint32 activity=0;
    uint32 act_led_act_sel_val=0, act_led_act_sel_mask=0;

    rc = BpGetAllAdvLedInfo(led_info, &cnt);
    if (rc != BP_SUCCESS )
    {
        //pr_err("Error reading Led Advanced info from board params\n");
        return;
    }

#if  defined(_BCM963158_) || defined(CONFIG_BCM963158)
    if( UtilGetChipRev() == 0xA0 )
    {
        /* to work around an issue with GPHY3 led we need to set the  qgphy3_led_ovrd to 1 */
        ETHSW_REG->crossbar_switch_ctrl |= ((ETHSW_REG->crossbar_switch_ctrl)&~(ETHSW_QGPHY3_LED_OVRD_MASK))|ETHSW_QGPHY3_LED_OVRD_MASK;
    }
#endif

#ifdef DEBUG_PHYS_INTF
    printk("total ports to configure %d\n", cnt);
#endif
    for(index=0;index < cnt; index++)
    {
        led_cfg=NULL;
#ifdef DEBUG_PHYS_INTF
    printk("intfType %d \n", led_info[index].pIntf->intfType);
#endif

        if(led_info[index].pIntf->intfType ==  BP_INTF_TYPE_xMII ||
            led_info[index].pIntf->intfType == BP_INTF_TYPE_GPHY ||
            led_info[index].pIntf->intfType == BP_INTF_TYPE_SGMII)
        {
            port=led_info[index].pIntf->portNum;
#if !defined(ETHSW_REG)
            if(port == 0)
                led_cfg=&(SYSPORT(0)->SYSTEMPORT_LED_REG);
            else if(port == 1)
                led_cfg=&(SYSPORT(1)->SYSTEMPORT_LED_REG);
#else
            if(port != SF2_WAN_PORT_NUM)
            {
#if defined(ETHSW_REG)
                led_cfg=&ETHSW_REG->led_ctrl[port];
#endif
            }
            else
            {
#if defined(XPORT_REG)
                /* crossbar WAN port use XPORT second port led config */
                led_cfg=&XPORT_REG->xport_led_cfg[1];
#else
                led_cfg=NULL;
#endif
            }
#endif
        }
        else if(led_info[index].pIntf->intfType == BP_INTF_TYPE_xPON) 
        {
            ///set to the XPON led control
            port=led_info[index].pIntf->portNum;
#if defined(XPORT_REG)
            if(port < sizeof(XPORT_REG->xport_led_cfg)/sizeof(LED_CFG))
            {
                led_cfg=&XPORT_REG->xport_led_cfg[port];
            }
#else
                led_cfg=NULL;
#endif
        }
        if(led_cfg)
        {
            int j;

            sel_10g_encode = 0;
            sel_1000m_encode = 0;
            sel_100m_encode = 0;
            sel_2500m_encode = 0;
            sel_10m_encode = 0;

            m10g_encode = 7;
            m2500_encode = 7;
            m1000_encode = 7;
            m100_encode = 7;
            m10_encode = 7;

            sel_value=0;
            encode_value=0;
            sel_mask=LINK_AND_SPEED_ENCODING_SEL_SEL_10M_ENCODE_MASK |
                         LINK_AND_SPEED_ENCODING_SEL_SEL_100M_ENCODE_MASK |
                         LINK_AND_SPEED_ENCODING_SEL_SEL_10G_ENCODE_MASK |
                         LINK_AND_SPEED_ENCODING_SEL_SEL_2500M_ENCODE_MASK |
                         LINK_AND_SPEED_ENCODING_SEL_SEL_1000M_ENCODE_MASK;

            encode_mask=LINK_AND_SPEED_ENCODING_M10G_ENCODE_MASK |
                         LINK_AND_SPEED_ENCODING_M10_ENCODE_MASK |
                         LINK_AND_SPEED_ENCODING_M1000_ENCODE_MASK |
                         LINK_AND_SPEED_ENCODING_M100_ENCODE_MASK |
                         LINK_AND_SPEED_ENCODING_M2500_ENCODE_MASK;

            act_tx_rx_mask = TX_ACT_EN_MASK|RX_ACT_EN_MASK;
            act_tx_rx_value = (led_cfg->led_ctrl&act_tx_rx_mask); 

            act_led_act_sel_val=0;
            act_led_act_sel_mask=SPDLNK_LED2_ACT_SEL_MASK|SPDLNK_LED1_ACT_SEL_MASK|SPDLNK_LED0_ACT_SEL_MASK|ACT_LED_ACT_SEL_MASK;
 
            //only handle netLinkLed0, netLinkLed1 and netLinkLed2
            // the activity led is handled separately only for RX/TX activity 
            for (j = 0; j < (MAX_LEDS_PER_PORT - 1); j++)
            {
                uint32_t led_mux = led_info[index].SpeedLed[j] & BP_NET_LED_SPEED_MASK;
                uint32_t led_activity = led_info[index].ActivityLed[j] & BP_NET_LED_SPEED_MASK;
                if(led_info[index].LedSettings[j] != BP_NOT_DEFINED )
                    act_tx_rx_value = led_info[index].LedSettings[j] & BP_NET_LED_SETTINGS_MASK;

#ifdef DEBUG_PHYS_INTF
    printk("port %d led_mux %x led_activity %x led_settings %d\n", port, led_mux, led_activity, act_tx_rx_value);
#endif

                if(led_mux != BP_NOT_DEFINED)
                {

                    if (led_mux & BP_NET_LED_SPEED_10G)
                        m10g_encode &= ~(1<<j);
                    if (led_mux & BP_NET_LED_SPEED_2500)
                        m2500_encode &= ~(1<<j);
                    if (led_mux & BP_NET_LED_SPEED_1G)
                        m1000_encode &= ~(1<<j);
                    if (led_mux & BP_NET_LED_SPEED_100)
                        m100_encode &= ~(1<<j);
                    if (led_mux & BP_NET_LED_SPEED_10)
                        m10_encode &= ~(1<<j);
                }
                if(led_activity != BP_NOT_DEFINED)
                {
                    if (led_activity & BP_NET_LED_SPEED_10G)
                        sel_10g_encode |= (1<<j);
                    if (led_activity & BP_NET_LED_SPEED_2500)
                        sel_2500m_encode |= (1<<j);
                    if (led_activity & BP_NET_LED_SPEED_1G)
                        sel_1000m_encode |= (1<<j);
                    if (led_activity & BP_NET_LED_SPEED_100)
                        sel_100m_encode |= (1<<j);
                    if (led_activity & BP_NET_LED_SPEED_10)
                        sel_10m_encode |= (1<<j);
                }
                /* to configure the speed led to show activity only for specified
                   speeds */
                if (led_activity && !(led_mux & led_activity))
                    activity = 1;

                 switch (j)
                {
                    case 0:
                        act_led_act_sel_val |= (activity<<SPDLNK_LED0_ACT_SEL_SHIFT);
                        break;
                    case 1:
                        act_led_act_sel_val |= (activity<<SPDLNK_LED1_ACT_SEL_SHIFT);
                        break;
                    case 2:
                        act_led_act_sel_val |= (activity<<SPDLNK_LED2_ACT_SEL_SHIFT);
                        break;
                }
            }

            if (!(led_info[index].SpeedLed[3] & BP_NET_LED_SPEED_MASK))
                act_led_act_sel_val |= (1<<ACT_LED_ACT_SEL_SHIFT);

            if(led_info[index].LedSettings[3] != BP_NOT_DEFINED )
                act_tx_rx_value=led_info[index].LedSettings[3] & BP_NET_LED_SETTINGS_MASK;

            sel_value=(sel_10g_encode << LINK_AND_SPEED_ENCODING_SEL_SEL_10G_ENCODE_SHIFT) |
                    (sel_2500m_encode<< LINK_AND_SPEED_ENCODING_SEL_SEL_2500M_ENCODE_SHIFT) |
                    (sel_1000m_encode << LINK_AND_SPEED_ENCODING_SEL_SEL_1000M_ENCODE_SHIFT)| 
                    (sel_100m_encode  << LINK_AND_SPEED_ENCODING_SEL_SEL_100M_ENCODE_SHIFT)|
                    (sel_10m_encode << LINK_AND_SPEED_ENCODING_SEL_SEL_10M_ENCODE_SHIFT);


            encode_value=(m10g_encode<< LINK_AND_SPEED_ENCODING_M10G_ENCODE_SHIFT) |
                    (m2500_encode << LINK_AND_SPEED_ENCODING_M2500_ENCODE_SHIFT) |
                    (m1000_encode << LINK_AND_SPEED_ENCODING_M1000_ENCODE_SHIFT) |
                    (m100_encode  << LINK_AND_SPEED_ENCODING_M100_ENCODE_SHIFT) |
                    (m10_encode << LINK_AND_SPEED_ENCODING_M10_ENCODE_SHIFT);


#ifdef DEBUG_PHYS_INTF
                printk("Encode m10_encode %x, m100_encode %x , m1000_encode %x \n", m10_encode, m100_encode, m1000_encode); 
                printk("SelEncode sel m10_encode %x, sel_m100_encode %x , sel_m1000_encode %x \n", sel_10m_encode, sel_100m_encode, sel_1000m_encode); 
                printk("sel_value %x, encode_value %x\n", sel_value, encode_value); 
#endif

            led_cfg->led_ctrl = (led_cfg->led_ctrl&(~act_tx_rx_mask)) | act_tx_rx_value; 
            led_cfg->led_ctrl |= (led_cfg->led_ctrl&(~act_led_act_sel_mask)) | act_led_act_sel_val; 
            led_cfg->led_encoding_sel = (led_cfg->led_encoding_sel&(~sel_mask)) | sel_value; 
            led_cfg->led_encoding = (led_cfg->led_encoding & (~encode_mask))|encode_value;
        }
    }

    /* aggregate LED setting */
    if (BpGetAggregateLnkLedGpio(&lnkAggrLed) == BP_SUCCESS &&
        BpGetAggregateActLedGpio(&actAggrLed) == BP_SUCCESS ) 
    {
#if defined(_BCM963158_) || defined(CONFIG_BCM963158) || defined(_BCM963178_) || defined(CONFIG_BCM963178)
        /* enable all 5 GPHY ports for aggregated led*/ 
        ETHSW_REG->aggregate_led_ctrl |= 0x1f;
#endif
    }
}
