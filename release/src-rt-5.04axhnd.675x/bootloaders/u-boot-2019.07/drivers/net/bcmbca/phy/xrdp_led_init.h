// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    
*/
/*
 * xrdp_led_init.h
 *
 *  Created on: April 2019
 *      Author: samyon.furman@broadcom.com
 */

#ifndef _XRDP_LED_INIT_H_
#define _XRDP_LED_INIT_H_

#if !defined(XRDP_LED)
static inline
#endif
int xrdp_leds_init(void * _leds_info)
#if !defined(XRDP_LED)
{ return 0; }
#endif
;

#endif
