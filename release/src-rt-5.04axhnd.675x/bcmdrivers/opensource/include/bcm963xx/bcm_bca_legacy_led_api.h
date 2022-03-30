/*
<:copyright-BRCM:2019:DUAL/GPL:standard 

   Copyright (c) 2019 Broadcom 
   All Rights Reserved

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

#ifndef __BCM_BCA_LEGACY_LED_API_H
#define __BCM_BCA_LEGACY_LED_API_H

#if defined(CONFIG_BCM_BCA_LEGACY_LED_API)

typedef enum
{   
    kLedAdsl,
    kLedSecAdsl,
    kLedWanData,
    kLedSes,
    kLedVoip,
    kLedVoip1,
    kLedVoip2,
    kLedPots,
    kLedDect,
    kLedGpon,
    kLedOpticalLink,
    kLedUSB,
    kLedSim,
    kLedSimITMS,
    kLedEpon,
    kLedWL0,
    kLedWL1,
    kLedEnd                             // NOTE: Insert the new led name before this one.
} BOARD_LED_NAME;

typedef enum
{
    kLedStateOff,                        /* turn led off */
    kLedStateOn,                         /* turn led on */
    kLedStateFail,                       /* turn led on red */
    kLedStateSlowBlinkContinues,         /* slow blink continues at 2HZ interval */
    kLedStateFastBlinkContinues,         /* fast blink continues at 4HZ interval */
    kLedStateUserWpsInProgress,          /* 200ms on, 100ms off */
    kLedStateUserWpsError,               /* 100ms on, 100ms off */
    kLedStateUserWpsSessionOverLap       /* 100ms on, 100ms off, 5 times, off for 500ms */                     
} BOARD_LED_STATE;

typedef enum 
{
    kLedOK,
    kLedFail,
} BOARD_LED_TYPE;

void kerSysLedCtrl(BOARD_LED_NAME ledName, BOARD_LED_STATE ledState);
unsigned int kerSysGetWifiLed(unsigned char core);
void kerSysWifiLed(unsigned int led, unsigned int on);

#ifndef __cplusplus
#include <linux/kconfig.h>
#include <linux/of.h>
int bca_legacy_led_request_sw_led(struct device_node *dn, const char *consumer_led_name, BOARD_LED_NAME led_name, BOARD_LED_TYPE led_type);
#endif

#endif
#endif
