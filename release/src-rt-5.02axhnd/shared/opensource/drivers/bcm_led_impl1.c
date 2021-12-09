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

#include "boardparms.h"
#include "bcm_led.h"
#include "bcm_pinmux.h"
#include "shared_utils.h"
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
 future: bcm_led_driver_brightness(number, brighness) -- 0-255
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
    unsigned char brightness;
    unsigned char is_hw_led;
    // glow_mode will go here eventually
} bcm_led_driver_state[LED_NUM_LEDS];

static int serial_is_inverted = 0;
static int old_led_ctrl = 0x0;
static volatile uint32* LedData;

static void led_do_set(unsigned short num) {
    unsigned short val;
    int al = 0;
    //int current;
    int serialinvert = 0;

    val =  bcm_led_driver_state[num].led_state;
    if( old_led_ctrl ) {
        al =  ((bcm_led_driver_state[num].led_bp & BP_ACTIVE_MASK) == BP_ACTIVE_LOW) ? 1 : 0;
        serialinvert =  (serial_is_inverted && (bcm_led_driver_state[num].led_bp & BP_GPIO_SERIAL)) ? 1 : 0;
    }

    // current = LED->mask & LED_NUM_TO_MASK(num);
    // FIXME - If brightness not 100%, need to compute both brightness and value every time
    if ( bcm_led_driver_state[num].is_hw_led ) {
        // HW LEDs have HW disabled when not in the ON state
        if (val) {
            *LedData |= LED_NUM_TO_MASK(num);
            LED->hWLedEn |= LED_NUM_TO_MASK(num);
        } else {
            *LedData &= ~LED_NUM_TO_MASK(num);
            LED->hWLedEn &= ~LED_NUM_TO_MASK(num);
        }
    } else {
        val = val ^ al ^ serialinvert ;
#ifdef BCM_LED_DEBUG
        printk("LED %d val %d after inversion by al %d invert %d\n",num,val,al,serialinvert);
#endif
        if (val) {
            *LedData |= LED_NUM_TO_MASK(num);
        } else {
            *LedData &= ~LED_NUM_TO_MASK(num);
        }
    }
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
        if ( bcm_led_driver_state[led].is_hw_led ) {
            // HW LEDs just toggle
            *LedData = *LedData ^ LED_NUM_TO_MASK(led);
            LED->hWLedEn &= ~LED_NUM_TO_MASK(led);
        } else {
            bcm_led_driver_state[led].led_state =  bcm_led_driver_state[led].led_state ^ 1;
            bcm_led_driver_state[led].led_bp = num;
            led_do_set(led);
        }
    }
}

void bcm_common_led_init(void) {
    int i;
    int j;
    unsigned short gpio;
    unsigned int brightwords[16];
    unsigned char *bright;
    uint16_t lnkLed, actLed;

    /* 63138/63381 A0/A1 use old led controller*/
    if ( ((CHIP_FAMILY_ID_HEX == 0x63138) || (CHIP_FAMILY_ID_HEX == 0x63381)) && ((UtilGetChipRev()&0xf0) == 0xa0) ) {
        old_led_ctrl = 1;
        LedData = &LED->mask;
    } else {
        old_led_ctrl = 0;
        LedData = &LED->SwData;
    }

    if ( old_led_ctrl == 0 && (CHIP_FAMILY_ID_HEX == 0x63138 || CHIP_FAMILY_ID_HEX == 0x63148) ) {
        /* set the software and hardware led input polarity. we controll the led polarity in parallel 
           and serial output polarity based on the AL and AH value from board parameter */
        LED->SwPolarity = (uint32)(-1);  //software input is always active high. This is hardware default too 
        LED->HwPolarity = 0xb8000; // hardware input is active low except bit 15 and 19 for adsl activity AH, bit 16 and 17 PWR LED AH  

        /* aggregate LED setting for 63138 */
        if (CHIP_FAMILY_ID_HEX == 0x63138 )
        {
            if (BpGetAggregateLnkLedGpio(&lnkLed) == BP_SUCCESS && 
                BpGetAggregateActLedGpio(&actLed) == BP_SUCCESS ) 
            {
                /* enable aggregate led input to led controller */
                lnkLed = lnkLed&BP_GPIO_NUM_MASK;
                actLed = actLed&BP_GPIO_NUM_MASK;
                LED->mask |= (0x1<<lnkLed)|(0x1<<actLed);
                /* hardware input polarity is active high */
                LED->HwPolarity |= (0x1<<lnkLed)|(0x1<<actLed);
            }
        }
    }

    bright = (unsigned char *)&brightwords;
    if( old_led_ctrl ) {
        if ((BP_SUCCESS == BpGetSerialLedData(&gpio)) && ((gpio & BP_ACTIVE_MASK) == BP_ACTIVE_LOW)) {
            LED->glbCtrl = 0x02; // Serial LEDs are not inverted
            serial_is_inverted = 0;
        } else {
            LED->glbCtrl = 0x00; // Serial LEDs are inverted
            serial_is_inverted = 1;
        }
    } else {
        serial_is_inverted = 0;
        LED->glbCtrl = 0xa; // init to ser led enable AH, failing clk clock edge parity and ser led output parity AH
        if ((BP_SUCCESS == BpGetSerialLedData(&gpio)) && ((gpio & BP_ACTIVE_MASK) == BP_ACTIVE_LOW)) 
            LED->glbCtrl = 0x8; //ser led output AL
    }

    for (i = 0; i < LED_NUM_LEDS/8 ; i++) {
        LED->brightCtrl[i] = 0x88888888;
    }
    for (i = 0; i < LED_NUM_LEDS ; i++) {
        bcm_led_driver_state[i].led_state = 0;
        bcm_led_driver_state[i].brightness = 255;
        bcm_led_driver_state[i].is_hw_led = 0;
    }
    for (j = 0 ; j < 32 ; j++) {
        bright[31-j] = bright[32+j] = 127 -  (j * j * 128 / (32 * 32) );
    }
#ifdef LED_NUM_PWM_LEDS
    for (i = 0; i < LED_NUM_PWM_LEDS ; i++) {
        for (j = 0 ; j < 16 ; j++) {
            LED->pledLut[i][j] = brightwords[j];
        }
    }
#endif
    if ((CHIP_FAMILY_ID_HEX == 0x6836) || 
        ((CHIP_FAMILY_ID_HEX == 0x6858) && ((UtilGetChipRev() & 0xf0) == 0xb0)) ||
        (CHIP_FAMILY_ID_HEX == 0x6856))
    {
        LED->HwPolarity = 0xc0000000; /* Ajust initial polarity state */
    }
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
                }

                if (LED->hWLedEn & LED_NUM_TO_MASK(led)) {
                    bcm_led_driver_state[led].is_hw_led = 1;
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
    if( old_led_ctrl == 0 ) {
       /* setup output polarity */
       LED->ParallelLedPolarity = parallel;
       LED->SerialLedPolarity = serial;
#ifdef BCM_LED_DEBUG
       printk("parallel 0x%x serial 0x%x\n", parallel, serial);
#endif
    }

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

#if defined(CONFIG_BCM963158) || defined(_BCM963158_)

//only enable it for CFE
//#define DEBUG_PHYS_INTF

void gen2_leds_init(void)
{
    PHYS_INTF_ADV_LEDS_INFO led_info[BP_MAX_PHYS_INTF_PORTS];
    int port, cnt=BP_MAX_PHYS_INTF_PORTS, index;
    int rc;
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


    /* to work around an issue with GPHY3 led we need to set the  qgphy3_led_ovrd to 1 */
    ETHSW_REG->crossbar_switch_ctrl |= ((ETHSW_REG->crossbar_switch_ctrl)&~(ETHSW_QGPHY3_LED_OVRD_MASK))|ETHSW_QGPHY3_LED_OVRD_MASK;

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
            if(port != SF2_WAN_PORT_NUM)
            {
                led_cfg=&ETHSW_REG->led_ctrl[port];
            }
            else
            {
                /* crossbar WAN port use XPORT second port led config */
                led_cfg=&XPORT_REG->xport_led_cfg[1];
            }
        }
        else if(led_info[index].pIntf->intfType == BP_INTF_TYPE_xPON) 
        {
            ///set to the XPON led control
            port=led_info[index].pIntf->portNum;
            if(port < sizeof(XPORT_REG->xport_led_cfg)/sizeof(LED_CFG))
            {
                led_cfg=&XPORT_REG->xport_led_cfg[port];
            }
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
                if((led_info[index].LedSettings[j] & BP_NET_LED_SETTINGS_MASK) != BP_NOT_DEFINED )
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

            if (!(led_info[port].SpeedLed[3] & BP_NET_LED_SPEED_MASK))
                act_led_act_sel_val |= (1<<ACT_LED_ACT_SEL_SHIFT);

            if((led_info[index].LedSettings[3] & BP_NET_LED_SETTINGS_MASK) != BP_NOT_DEFINED )
                act_tx_rx_value=led_info[3].LedSettings[3] & BP_NET_LED_SETTINGS_MASK;

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

}
#else
/* default leds configuration:
 * LED0:On for each speed
 * LED1:On when link up and blinks on activity
 */

#if defined(_BCM963138_) || defined(CONFIG_BCM963138) || defined(_BCM963148_) || defined(CONFIG_BCM963148) || defined(_BCM94908_) || defined(CONFIG_BCM94908) || defined(_BCM963158_) || defined(CONFIG_BCM963158)
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

void bcm_ethsw_set_led(void)
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
#endif


#endif

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
