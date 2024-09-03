/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard
    
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
/*
 * xrdp_led_init.h
 *
 *  Created on: April 2019
 *      Author: samyon.furman@broadcom.com
 */

#ifndef _XRDP_LED_INIT_H_
#define _XRDP_LED_INIT_H_

#if defined(EPHY_LED)
int ephy_leds_init(void * _leds_info);
#define xrdp_leds_init ephy_leds_init
#else
#if !defined(XRDP_LED)
static inline
#endif
int xrdp_leds_init(void * _leds_info)
#if !defined(XRDP_LED) 
{ return 0; }
#endif
;
#endif

#endif
