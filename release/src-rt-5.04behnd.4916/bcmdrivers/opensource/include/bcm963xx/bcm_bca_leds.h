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

#ifndef __BCM_BCA_LEDS_H
#define __BCM_BCA_LEDS_H

#if defined(CONFIG_BCM_BCA_LED)

#include <linux/of.h>

typedef struct _bca_leds_info_t
{
    uint32_t port_id;
    uint32_t skip_in_aggregate;
    uint32_t link[4];
    uint32_t activity[4];
    uint32_t sw_id;
}bca_leds_info_t;

struct led_classdev *bca_led_request_sw_led(struct device_node *dn, const char *consumer_led_name);
void bca_led_request_network_leds(struct device_node *dn, bca_leds_info_t *leds_info);

#endif
#endif
