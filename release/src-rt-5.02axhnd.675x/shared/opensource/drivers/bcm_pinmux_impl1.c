/*
 * <:copyright-BRCM:2013:DUAL/GPL:standard
 * 
 *    Copyright (c) 2013 Broadcom 
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

/*
  These are low level functions that can be called from CFE or from the Linux init code.
  If pinmux changes are needed after Linux init, support should be added to the board 
  driver including any necessary locking.
*/

#include "boardparms.h"
#include "bcm_pinmux.h"
#include "bcm_led.h"

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



void bcm_set_pinmux(unsigned int pin_num, unsigned int mux_num)
{
    unsigned int tp_blk_data_lsb;
    //printk("set pinmux %d to %d\n",pin_num, mux_num);

    tp_blk_data_lsb= 0;
    tp_blk_data_lsb |= pin_num;
    tp_blk_data_lsb |= (mux_num << PINMUX_DATA_SHIFT);
    GPIO->TestPortBlockDataMSB = 0;
    GPIO->TestPortBlockDataLSB = tp_blk_data_lsb;
    GPIO->TestPortCmd = LOAD_MUX_REG_CMD;
}

void bcm_init_pinmux_interface(unsigned int interface) {
    int i, n, errcnt;
    unsigned short Function[BP_PINMUX_MAX];
    unsigned int Muxinfo[BP_PINMUX_MAX];

    if (BP_SUCCESS != BpGetIfacePinmux (interface, BP_PINMUX_MAX, &n, &errcnt, Function, Muxinfo)) {
        return;
    }
    for (i = n-1 ; 0 <= i ; i--) {
        bcm_set_pinmux( Muxinfo[i] & BP_PINMUX_PIN_MASK, (Muxinfo[i] & BP_PINMUX_VAL_MASK) >> BP_PINMUX_VAL_SHIFT );
    }
}

int bcm_init_pinmux(void)
{
    int i, j, n, errcnt, op;
    int lednum;
    unsigned int serial = 0;
    unsigned int ledsrc = 0;
    unsigned short Function[BP_PINMUX_MAX];
    unsigned int Muxinfo[BP_PINMUX_MAX];
    short *optled_map;
    optled_map = bcm_led_driver_get_optled_map();

    if (BP_SUCCESS != BpGetAllPinmux (BP_PINMUX_MAX, &n, &errcnt, Function, Muxinfo)) {
        return 0;
    }

    for (i = n-1 ; 0 <= i ; i--) {
        lednum = Function[i] & BP_GPIO_NUM_MASK;
        if (Muxinfo[i] & BP_PINMUX_OPTLED_VALID) {
            j = (Muxinfo[i] & BP_PINMUX_OPTLED_MASK) >> BP_PINMUX_OPTLED_SHIFT;
            optled_map[lednum] = j;
            lednum = j;
        } else {
            optled_map[lednum] = lednum;
        }
        op = Muxinfo[i] & BP_PINMUX_OP_MASK;
        if (Function[i] & BP_GPIO_SERIAL) {
            serial |= 1 << (Function[i] & BP_GPIO_NUM_MASK);
            bcm_led_zero_flash_rate(Function[i] & BP_GPIO_NUM_MASK);
        } else {
            bcm_set_pinmux( Muxinfo[i] & BP_PINMUX_PIN_MASK, (Muxinfo[i] & BP_PINMUX_VAL_MASK) >> BP_PINMUX_VAL_SHIFT );
            if ((op == BP_PINMUX_HWLED) || (op == BP_PINMUX_SWLED)) {
                // zero the flash rate for any LED that needs to be on/off
                bcm_led_zero_flash_rate(lednum);
            }
        }
        if ((op == BP_PINMUX_HWLED) || (op == BP_PINMUX_PWMLED)) {
            ledsrc |= 1 << lednum;
        }
    }
    // count bits enabled for serial and add additional to byte boundary
    n = 0; 
    for (i = 0 ; i < 32 ; i++) {
        if (serial & (1 << i)) {
            n++; 
        }
    }
    i = 31;
    while (n % 8) {
        if (serial & (1 << i)) {
            printk("ERROR: could not append enough serial LEDs\n");
        } else {
            serial |= (1<<i);
        }
        i--;
        n++;
    }

    bcm_led_set_source(serial, ledsrc);

    return 0;
}

#ifndef _CFE_ 
EXPORT_SYMBOL(bcm_set_pinmux );
#endif
