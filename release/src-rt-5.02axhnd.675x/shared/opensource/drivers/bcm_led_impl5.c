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
#include "bcm_led.h"
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

//#define BCM_LED_DEBUG 1

/*
  These are low level functions that can be called from CFE or from the Linux board driver
  The Linux board driver handles any necessary locking so these functions should not be called
  directly from elsewhere.
*/

/* 
 bcm_led_driver_set(number, state) -- on/off
 bcm_led_driver_toggle(number) 
 future: bcm_led_driver_brightness(number, brightness) -- 0-255
 future: bcm_led_driver_map_glow(number, state, waveform[])

 For now, 
    stete 0 = off
    state 1 = on
 This will be replaced with a more flexible set of states, preserving 0 and 1

*/

static short g_optled_map[BP_PINMUX_MAX];

static struct bcm_led_driver_state {
    unsigned short led_state;
    unsigned short led_bp;
    unsigned short brightness;
    unsigned char is_hw_led;
    unsigned short real_led_ch;
} bcm_led_driver_state[LED_NUM_LEDS];

static void led_do_set(unsigned short num)
{
    unsigned short val;
    unsigned short real_led_ch = bcm_led_driver_state[num].real_led_ch;

    val =  bcm_led_driver_state[num].led_state;

    if ( bcm_led_driver_state[num].is_hw_led )
    {
        if (val)
            LED->hWLedEn &= ~LED_NUM_TO_MASK(real_led_ch);
        else
            LED->hWLedEn |= LED_NUM_TO_MASK(real_led_ch);
    }

    if (val)
        LED->SwSet = LED_NUM_TO_MASK(real_led_ch);
    else
        LED->SwClear = LED_NUM_TO_MASK(real_led_ch);
}

/* bcm_led_driver_get_optled_map()
   permits pinmux init code to get a pointer to the optled_map.  Any time that pinmux init code 
   creates a mapping where a specific GPIO NUMBER maps to an LED register bit number. this
   map must be populated.  This includes direct mapping when, for example, GPIO3 maps to LED3
*/
short * bcm_led_driver_get_optled_map(void)
{
    return(g_optled_map);
}

void bcm_led_driver_set(unsigned short num, unsigned short state)
{
    unsigned short led;
    unsigned short gpio_state;
    
#ifdef BCM_LED_DEBUG
    printk("LED %x set state %d\n",num,state);
#endif

    if (num & BP_LED_USE_GPIO)
    {
        if (((num & BP_ACTIVE_LOW) && (state == BCM_LED_ON)) ||
            (!(num & BP_ACTIVE_LOW) && (state == BCM_LED_OFF)))
            gpio_state = 0;
        else
            gpio_state = 1;

        bcm_gpio_set_dir(num, 1);
        bcm_gpio_set_data(num, gpio_state);
    }
    else
    {
        led = num & BP_GPIO_NUM_MASK;
        led = g_optled_map[led];
        bcm_led_driver_state[led].led_state = state;
        bcm_led_driver_state[led].led_bp = num;
        led_do_set(led);
    }
}

void bcm_led_driver_toggle(unsigned short num)
{
    unsigned short led;
    led = num & BP_GPIO_NUM_MASK;
    
    if (num & BP_LED_USE_GPIO)
    {
        bcm_gpio_set_dir(num, 1);
        bcm_gpio_set_data(num, GPIO_NUM_TO_MASK(num)^bcm_gpio_get_data(num));
    }
    else
    {    
        led = g_optled_map[led];
        bcm_led_driver_state[led].led_state =  bcm_led_driver_state[led].led_state ^ 1;
        bcm_led_driver_state[led].led_bp = num;
        led_do_set(led);
    }
}

void bcm_led_zero_flash_rate(int channel)
{
    if( channel < LED_NUM_LEDS )
    {
        LED->LedCfg[channel].cfg0.Bits.flash_ctrl = 0;
        LED->ChActivate = LED_NUM_TO_MASK(channel);
    }
    return;
}

void bcm_led_set_source(unsigned int serial_sel, unsigned int hwled_sel)
{
    int i,j;

    for(i = 0; i<8; i++)
    {
#ifdef BCM_LED_DEBUG
        printk("\nOutMux[%d]: 0x", i);
#endif
        for (j=0; j<4; j++)
        {
#ifdef BCM_LED_DEBUG
            printk("%c%02x ",bcm_led_driver_state[i*4 + j].is_hw_led ? '*':' ',
                bcm_led_driver_state[i*4 + j].real_led_ch);
#endif
            LED->OutMux[i] &= ~((0x1f)<<(j*8));
            LED->OutMux[i] |= (bcm_led_driver_state[i*4 + j].real_led_ch)<<(j*8);
        }
    }

    LED->serialLedShiftSel = serial_sel;
    LED->hWLedEn = hwled_sel;

    return;
}

void bcm_common_led_init(void) {

    unsigned short order;
    unsigned short gpio;
    int i = 0;
    unsigned int glbCtrl = LED->glbCtrl;

    glbCtrl &= ~0x1a; /* Clean bits */
    if ((BP_SUCCESS == BpGetSerialLedData(&gpio)) && ((gpio & BP_ACTIVE_MASK) == BP_ACTIVE_LOW)) 
        glbCtrl |= 0x8; //ser led output AL
    else 
        glbCtrl |= 0xa; //ser led output AH

    if ((BP_SUCCESS == BpGetSerialLedShiftOrder(&order)) && (order == BP_SERIAL_LED_SHIFT_MSB_FIRST)) 
        glbCtrl |= 0x10;

    LED->glbCtrl = glbCtrl;

    for (i = 0; i < LED_NUM_LEDS ; i++) {
        bcm_led_driver_state[i].led_state = 0;
        bcm_led_driver_state[i].brightness = 255;
    }

    return;
}

void bcm_common_led_setAllSoftLedsOff(void)
{
    unsigned short gpio;
    unsigned short led;
    unsigned int parallel = 0;
    unsigned int serial = 0;
    int i = 0, rc;
    void* token = NULL;

    for(;;)
    {
        rc = BpGetLedGpio(i, &token, &gpio);
        if( rc == BP_MAX_ITEM_EXCEEDED )
            break;
        else if( rc == BP_SUCCESS && gpio != BP_GPIO_NONE )
        {
            if (gpio & BP_LED_USE_GPIO)
            {
                bcm_led_driver_set(gpio, BCM_LED_OFF);
            }
            else
            {
                led = gpio & BP_GPIO_NUM_MASK;
                led = g_optled_map[led];

                switch (gpio & (BP_ACTIVE_MASK | BP_GPIO_SERIAL)) {
                case (BP_ACTIVE_LOW | BP_GPIO_SERIAL):
                    serial &= ~LED_NUM_TO_MASK(led);
                    break;
                case (BP_ACTIVE_HIGH | BP_GPIO_SERIAL):
                    serial |= LED_NUM_TO_MASK(led);
                    break;
                case (BP_ACTIVE_LOW):
                    parallel &= ~LED_NUM_TO_MASK(led);
                    break;
                case (BP_ACTIVE_HIGH):
                    parallel |= LED_NUM_TO_MASK(led);
                    break;
                default:
                    break;
                }

                if (LED->hWLedEn & LED_NUM_TO_MASK(led)) {
#ifdef BCM_LED_DEBUG
                    printk("off: LED %d is HW\n",led);
#endif
                } else {
                    bcm_led_driver_set( gpio, 0 );
#ifdef BCM_LED_DEBUG
                    printk("off: LED %d is OFF\n",led);
#endif
                  }
            }
        }
        else
        {
            token = 0;
            i++;
        }
    }
#ifdef BCM_LED_DEBUG
    for (i = 0; i < 64 ; i++) {
        if ((i & 0x7) == 0) {
            printk("\noptled_map %d:",i);
        }
        printk(" %4d",g_optled_map[i]);
    }
#endif

    /* setup output polarity */
    LED->ParallelLedPolarity = parallel;
    LED->SerialLedPolarity = serial;

#ifdef BCM_LED_DEBUG
    printk("\nparallel 0x%x serial 0x%x\n", parallel, serial);
#endif

    return;
}

void bcm_common_led_setInitial(void)
{
    unsigned short gpio;
    if( BpGetBootloaderPowerOnLedGpio( &gpio ) == BP_SUCCESS )
        bcm_led_driver_set( gpio, BCM_LED_ON );
    if( BpGetWanDataLedGpio( &gpio ) == BP_SUCCESS )
        bcm_led_driver_set( gpio, BCM_LED_OFF );
}

void bcm_cled_mux_leds(unsigned int gpio, unsigned int output_led, unsigned int input_led, unsigned int is_hw)
{
    static int is_first = 1;
    int i;

    if (is_first)
    {
        for (i = 0; i < LED_NUM_LEDS; i++)
        {
            bcm_led_driver_state[i].real_led_ch = i;
            bcm_led_driver_state[i].is_hw_led = 0;
        }
        is_first = 0;
    }

    if (bcm_led_driver_state[output_led].is_hw_led)
    {
        printk("Trying to remap the HW led , Failed.");
        return;
    }

    if (input_led != output_led)
    {
       unsigned int tmp = bcm_led_driver_state[output_led].real_led_ch;

       bcm_led_driver_state[output_led].real_led_ch = input_led;
       while (bcm_led_driver_state[input_led].is_hw_led)
           input_led = bcm_led_driver_state[input_led].real_led_ch;
       bcm_led_driver_state[input_led].real_led_ch = tmp;
       bcm_led_driver_state[input_led].is_hw_led = 0;
    }
    bcm_led_driver_state[output_led].is_hw_led = is_hw;

    if (is_hw)
    {
        uint16_t lnkLed, actLed;
        /* aggregate LEDs setting 
         The Hardware input polarity is Active High in differ of all others.*/

        if (BpGetAggregateLnkLedGpio(&lnkLed) == BP_SUCCESS)
        {
            if ((lnkLed & BP_GPIO_NUM_MASK) == gpio)
                LED->HwPolarity |= (0x1<<input_led);
        }
        if (BpGetAggregateActLedGpio(&actLed) == BP_SUCCESS)
        {
            if ((actLed & BP_GPIO_NUM_MASK) == gpio)
                LED->HwPolarity |= (0x1<<input_led);
        }
    }
}

#ifndef _CFE_ 
static int bcm_common_led_linux_init(void)
{
    bcm_common_led_init();
    bcm_common_led_setAllSoftLedsOff();
    bcm_common_led_setInitial();

    return 0;
}
subsys_initcall(bcm_common_led_linux_init);
#endif /* ! _CFE_ */
