/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard

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
