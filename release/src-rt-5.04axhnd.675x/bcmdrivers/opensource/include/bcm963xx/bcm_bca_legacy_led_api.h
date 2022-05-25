/*
<:copyright-BRCM:2019:DUAL/GPL:standard 

   Copyright (c) 2019 Broadcom 
   All Rights Reserved

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
