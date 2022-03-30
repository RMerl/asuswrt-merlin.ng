// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    
*/
/*
 * ephy_led_init.h
 *
 *  Created on: April 2019
 *      Author: samyon.furman@broadcom.com
 */

#ifndef _EPHY_LED_INIT_H_
#define _EPHY_LED_INIT_H_

#if !defined(EPHY_LED)
static inline
#endif
int ephy_leds_init(void * _leds_info)
#if !defined(EPHY_LED)
{ return 0; }
#endif
;

#endif
