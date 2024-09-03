/*
   <:copyright-BRCM:2015:DUAL/GPL:standard
   
      Copyright (c) 2015 Broadcom 
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

#include <linux/module.h>
#include <linux/init.h>
#include <linux/of.h>

#include "bcm_bca_leds.h"
#include "ephy_led_init.h"
#define DT_NUM_OF_PORTS 8
static bca_leds_info_t port_leds_info[DT_NUM_OF_PORTS];

void dt_parse_switch_ports(void)
{
    struct device_node *np, *child, *child_p;
    int port_index = 0;

    /* XXX: Until BP is removed, this tests if a root switch is configured with active ports */
    if ((np = of_find_compatible_node(NULL, NULL, "brcm,enet")))
    {
        for_each_available_child_of_node(np, child)
        {
            for_each_available_child_of_node(child, child_p)
		{
			printk("node %s\n", child_p->full_name);
			printk("node %s\n", child_p->name);
#if defined(CONFIG_BCM_BCA_LED)
			bca_led_request_network_leds(child_p, &port_leds_info[port_index]);
			printk("calling ephy_leds_init\n");
			ephy_leds_init(&port_leds_info[port_index]);
			port_index++;
#endif
		}
        }
    }

    if (np)
        of_node_put(np);

}

static int __init init_late_led(void)
{
	dt_parse_switch_ports();
	return 0;
}
late_initcall(init_late_led);


MODULE_DESCRIPTION("BCM internal ethernet dummy led driver");
MODULE_LICENSE("GPL");
