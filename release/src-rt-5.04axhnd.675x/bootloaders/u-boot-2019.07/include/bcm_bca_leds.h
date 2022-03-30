// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2019 Broadcom
 */
/*

*/

#ifndef __BCM_BCA_LEDCTRL_H
#define __BCM_BCA_LEDCTRL_H

#if defined(CONFIG_BCM_BCA_LED)

#include <dm.h>
#include <dm/ofnode.h>

typedef struct
{
    uint32_t port_id;
    uint32_t skip_in_aggregate;
    uint32_t link[4];
    uint32_t activity[4];
}bca_leds_info_t;

struct udevice * bca_led_request_sw_led(ofnode dn, const char *consumer_led_name);
void bca_led_request_network_leds(ofnode dn, bca_leds_info_t *leds_info);
#endif
#endif
