/*
   <:copyright-BRCM:2015:DUAL/GPL:standard
   
      Copyright (c) 2015 Broadcom 
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
