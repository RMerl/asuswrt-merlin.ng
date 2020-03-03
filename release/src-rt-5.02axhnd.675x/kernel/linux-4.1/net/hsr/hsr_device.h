/* Copyright 2011-2014 Autronica Fire and Security AS
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * Author(s):
 *	2011-2014 Arvid Brodin, arvid.brodin@alten.se
 */

#ifndef __HSR_DEVICE_H
#define __HSR_DEVICE_H

#include <linux/netdevice.h>
#include "hsr_main.h"

void hsr_dev_setup(struct net_device *dev);
int hsr_dev_finalize(struct net_device *hsr_dev, struct net_device *slave[2],
		     unsigned char multicast_spec);
void hsr_check_carrier_and_operstate(struct hsr_priv *hsr);
bool is_hsr_master(struct net_device *dev);
int hsr_get_max_mtu(struct hsr_priv *hsr);

#endif /* __HSR_DEVICE_H */
