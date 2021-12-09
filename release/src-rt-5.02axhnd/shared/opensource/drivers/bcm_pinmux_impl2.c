/*
 * <:copyright-BRCM:2014:DUAL/GPL:standard
 * 
 *    Copyright (c) 2014 Broadcom 
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

/*
  These are low level functions that can be called from CFE or from the Linux init code.
  If pinmux changes are needed after Linux init, support should be added to the board
  driver including any necessary locking.
*/

#include "boardparms.h"
#include "bcm_pinmux.h"
#include "bcm_gpio.h"

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

#define DUPLEX_LED      0
#define SPEED_LED_100   1
#define SPEED_LED_1000  2
#define LED_LAN         3

/*
 * Matches a Led definition in boardparms.c with a gpio signal.
 *
 * Each row defines the gpio mappings for a network interface:
 * pin_to_mux[0] -> leds for bp_ulPhyId = 0
 * pin_to_mux[1] -> leds for bp_ulPhyId = 1
 * ...
 * pin_to_mux[n] -> leds for bp_ulPhyId = n
 *
 * Each column defines a led type:
 *
 *  [ duplexLed  , speedLed100 , speedLed1000, LedLan  ]
 */
int pin_to_mux[][4] = {
    {BP_GPIO_NONE, BP_GPIO_NONE, BP_GPIO_NONE, BP_GPIO_NONE},  //(external gphy)
    {BP_GPIO_NONE, BP_GPIO_NONE, BP_GPIO_NONE, BP_GPIO_69_AH}, //(internal gphy)
    {BP_GPIO_NONE, BP_GPIO_NONE, BP_GPIO_NONE, BP_GPIO_NONE}   //(plc)
};

/****************************************************************
 * Name         : bcm_init_pinmux
 *
 * Description  : Walks the boardparms structure for the active
 *                board ID looking for led parameters. For each led
 *                parameter, it finds its gpio_mux match and calls
 *                bcm_set_pinmux to configure the GPIO mux.
 ****************************************************************/
int bcm_init_pinmux(void)
{
    int i;
    int pin;
    int iface_cnt;
    int iface_id;
    const ETHERNET_MAC_INFO *EnetInfo;

    EnetInfo = BpGetEthernetMacInfoArrayPtr();
    bitcount(iface_cnt, EnetInfo[0].sw.port_map);

    for (i = 0; i < iface_cnt; i++)
    {
        iface_id = EnetInfo[0].sw.phy_id[i];
        if ((pin = EnetInfo[0].sw.ledInfo[i].duplexLed) != BP_GPIO_NONE)
            bcm_set_pinmux(pin, pin_to_mux[iface_id][DUPLEX_LED]);
        if ((pin = EnetInfo[0].sw.ledInfo[i].speedLed100) != BP_GPIO_NONE)
            bcm_set_pinmux(pin, pin_to_mux[iface_id][SPEED_LED_100]);
        if ((pin = EnetInfo[0].sw.ledInfo[i].speedLed1000) != BP_GPIO_NONE)
            bcm_set_pinmux(pin, pin_to_mux[iface_id][SPEED_LED_1000]);
        if ((pin = EnetInfo[0].sw.ledInfo[i].LedLan) != BP_GPIO_NONE)
            bcm_set_pinmux(pin, pin_to_mux[iface_id][LED_LAN]);
    }
    return 0;
}

/****************************************************************
 * Name         : bcm_set_pinmux
 *
 * Description  : Given a pin number and a mux gpio, configures the GPIO
 *                mux to connect the pin to the internal gpio
 *
 * Parameters   : [IN] pin_num      - Pin number (physical GPIO)
 *                [IN] mux_num      - Mux GPIO
 *
 * Notes        : This function calls bcm_gpio_set_funcmode to configure
 *                GPIO registers. The caller function must handle both
 *                MUX _and_ GPIO spinlocks when calling this function.
 ****************************************************************/
void bcm_set_pinmux(unsigned int pin_num, unsigned int mux_num)
{
    volatile uint32 *GPIOMuxCtrl;
    int mux_ctrl;
    int mux_nsel;
    unsigned int tmp;

    if (pin_num < 4)
    {
        mux_ctrl = 0;
    } else
    {
        mux_ctrl = (pin_num / 4);
        if (mux_ctrl >= 8)
        {
            printk("Boardparms error: pin_num %d is too big\n", pin_num);
            return;
        }
    }

    mux_nsel = pin_num % 4;
    GPIOMuxCtrl = &(GPIO->GPIOMuxCtrl_0) + mux_ctrl;

    tmp = *GPIOMuxCtrl;
    tmp &= ~(GPIO_0_MUX_NSEL << (8 * mux_nsel));
    tmp |= (mux_num << (8 * mux_nsel));
    *GPIOMuxCtrl = tmp;
    bcm_gpio_set_funcmode(pin_num, GPIO_MUXED_MODE);
}

#ifndef _CFE_
arch_initcall(bcm_init_pinmux);
EXPORT_SYMBOL(bcm_set_pinmux);
#endif
