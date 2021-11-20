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

static void bcm_ethsw_set_led_reg(volatile uint32_t* ledctrl)
{
    uint32_t value;

    value = *ledctrl;

    /* turn off all the leds */
    value |= ETHSW_LED_CTRL_ALL_SPEED_MASK;
    
    /* broadcom reference design alway use LED_SPD0 for 1g link and LED_SPD1 for 100m link */
    value &= ~(ETHSW_LED_CTRL_SPEED_MASK << ETHSW_LED_CTRL_1000M_SHIFT);
    value |= (ETHSW_LED_CTRL_SPD0_ON << ETHSW_LED_CTRL_1000M_SHIFT)|(ETHSW_LED_CTRL_SPD1_OFF << ETHSW_LED_CTRL_1000M_SHIFT);

    value &= ~(ETHSW_LED_CTRL_SPEED_MASK<<ETHSW_LED_CTRL_100M_SHIFT);
    value |= (ETHSW_LED_CTRL_SPD0_OFF << ETHSW_LED_CTRL_100M_SHIFT)|(ETHSW_LED_CTRL_SPD1_ON << ETHSW_LED_CTRL_100M_SHIFT);

    *ledctrl = value;

    return;
}

void bcm_ethsw_led_init(void)
{
    volatile uint32_t* ledctrl;
    uint16_t lnkLed, actLed;
    int i;

    /* set the sw internal phy LED mode. the default speed mode encoding is wrong.
       apply to all 5 internal GPHY */
    for( i = 0; i < 5; i++ )
    {
#if defined(_BCM94908_) || defined(CONFIG_BCM94908) || defined(_BCM963158_) || defined(CONFIG_BCM963158)
        ledctrl = &ETHSW_REG->led_ctrl[i].led_encoding;
#else
        ledctrl = &ETHSW_REG->led_ctrl[i];
#endif
        bcm_ethsw_set_led_reg(ledctrl);
    }

    /* WAN led */
#if defined(_BCM94908_) || defined(CONFIG_BCM94908) || defined(_BCM963158_) || defined(CONFIG_BCM963158)
    ledctrl = &ETHSW_REG->led_wan_ctrl.led_encoding;
#else
    ledctrl = &ETHSW_REG->led_wan_ctrl;
#endif
    bcm_ethsw_set_led_reg(ledctrl);

    /* aggregate LED setting */
    if (BpGetAggregateLnkLedGpio(&lnkLed) == BP_SUCCESS &&
        BpGetAggregateActLedGpio(&actLed) == BP_SUCCESS ) 
    {
#if defined(_BCM94908_) || defined(CONFIG_BCM94908) || defined(_BCM963158_) || defined(CONFIG_BCM963158)
        /* link led polarity is reversed from hw.  Suppose to be active low but it is active high.
           change the polarity in led ctrl registr and also enable all 5 GPHY ports */ 
        ETHSW_REG->aggregate_led_ctrl |= (ETHSW_AGGREGATE_LED_CTRL_LNK_POL_SEL_MASK|0x1f); 
#endif
    }

    return;
}
