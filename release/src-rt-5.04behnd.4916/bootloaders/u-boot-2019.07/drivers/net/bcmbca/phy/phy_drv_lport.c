// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    
*/

/*
 *  Created on: Jan 2016
 *      Author: yuval.raviv@broadcom.com
 */

/*
 * Common functions for lport connected phy drivers
 */

#include "phy_drv.h"
#include "lport_intr.h"

static void lport_link_change_handler(phy_dev_t *phy_dev)
{
    uint32_t port = (uint64_t)phy_dev->priv;

    phy_dev_link_change_notify(phy_dev);
    lport_intr_enable(LPORT_PORT_LINK, port, 1);
}

static void lport_intr_cb(const lport_intr_info_s *info, void *priv)
{
    phy_dev_t *phy_dev = priv;
    uint32_t port = (uint64_t)phy_dev->priv;

    lport_intr_enable(LPORT_PORT_LINK, port, 0);
    lport_intr_clear(LPORT_PORT_LINK, port);

    phy_dev_queue_work(phy_dev, lport_link_change_handler);
}

int lport_link_change_register(phy_dev_t *phy_dev)
{
    int ret;
    uint32_t port = (uint64_t)phy_dev->priv;

    if ((ret = lport_intr_register(LPORT_PORT_LINK, port, lport_intr_cb, phy_dev)))
    {
        printk("Failed to register link interrupt for port %d\n", port);
        return ret;
    }

    lport_intr_enable(LPORT_PORT_LINK, port, 1);

    return 0;
}

int lport_link_change_unregister(phy_dev_t *phy_dev)
{
    uint32_t port = (uint64_t)phy_dev->priv;

    lport_intr_enable(LPORT_PORT_LINK, port, 0);
    lport_intr_unregister(LPORT_PORT_LINK, port);

    return 0;
}
