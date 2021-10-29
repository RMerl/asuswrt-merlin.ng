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

/*
 *  Created on: Jan/2016
 *      Author: ido@broadcom.com
 */

#include "port.h"
#include "enet.h"
#include "mux_index.h"
#include <bcm_OS_Deps.h>
#include <rdpa_api.h>
#include <rdpa_types.h>

        
static int add_wifi_port(struct net_device *dev)
{
    port_info_t port_info = { };
    rdpa_if rdpaif;
    enetx_port_t *p;
    int ssid;

    if (!strcmp(dev->name, "wl0"))
        rdpaif = rdpa_if_ssid0;
    else if (sscanf(dev->name, "wl0.%d", &ssid) == 1)
        rdpaif = rdpa_if_ssid0 + ssid;
    else if (!strcmp(dev->name, "wl1"))
        rdpaif = rdpa_if_ssid0 + WL_NUM_OF_SSID_PER_UNIT;
    else if (sscanf(dev->name, "wl1.%d", &ssid) == 1)
        rdpaif = rdpa_if_ssid0 + WL_NUM_OF_SSID_PER_UNIT + ssid;
    else
    {
        /* This is not a wifi interface, no need to demux it */
        return -1;
    }

    if (rdpaif > rdpa_if_ssid15)
    {
        enet_err("failed to parse correct rdpaif for %s %d\n", dev->name, rdpaif);
        return -1;
    }

    port_info.port = rdpaif;
    if (port_create(&port_info, root_sw, &p))
        return -1;
    
    p->dev = dev;
    /* note: wifi dev is not enetx_netdev so there is no priv holding port pointer */
    
    enet_dbg("added wifi port %s\n", p->obj_name);
    
    if (port_init(p))
    {
        sw_free(&p);
        return -1;
    }

    return 0;
}

static int wifi_notifier_call(struct notifier_block *nb, unsigned long event, void *_dev)
{
    struct net_device *dev = NETDEV_NOTIFIER_GET_DEV(_dev);

    switch (event)    
    {
    case NETDEV_REGISTER:
        add_wifi_port(dev);
        break;
    case NETDEV_UNREGISTER:
        /* TODO: remove port */
        break;
    }

    return 0;
}

static struct notifier_block nb =
{
    .notifier_call = wifi_notifier_call,
    .priority = 0,
};

int register_wifi_dev_forwarder(void)
{
    if (register_netdevice_notifier(&nb))
    {
        enet_err("register_netdevice_notifier() failed\n");
        return -1;
    }

    return 0;
}

static int port_runner_wifi_init(enetx_port_t *self)
{
    self->p.port_cap = PORT_CAP_LAN_ONLY;
#if defined(CONFIG_BLOG)
    self->n.blog_phy = BLOG_WLANPHY;
    self->n.blog_chnl = self->n.blog_chnl_rx = 0;
#endif

    /* in case unload/reload clear previous value first */
    mux_set_rx_index(root_sw, self->p.port_id, NULL);
    if (mux_set_rx_index(root_sw, self->p.port_id, self))
        return -1;

    return 0;
}

static int port_runner_wifi_uninit(enetx_port_t *self)
{
    unregister_netdevice_notifier(&nb);
    
    if (mux_set_rx_index(root_sw, self->p.port_id, NULL))
        return -1;

    return 0;
}

port_ops_t port_runner_wifi =
{
    .init = port_runner_wifi_init,
    .uninit = port_runner_wifi_uninit,
};

