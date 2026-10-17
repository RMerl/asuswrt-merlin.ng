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

/*
 *  Created on: Nov/2024
 *      Author: ido@broadcom.com
 */

#include "port.h"
#include "enet_dbg.h"

#define PORT_GROUP_MAX 5

static enetx_port_t *port_groups[PORT_GROUP_MAX];


int port_group_set(enetx_port_t *port)
{
    int port_group = port->port_info.port_group;

    if (port_group == -1)
        return 0;

    if (port_group >= PORT_GROUP_MAX)
    {
        enet_err("port_group_set: Invalid port group [%d] (%s)\n", port_group, port->obj_name);
        return -1;
    }

    if (port_groups[port_group])
    {
        enet_dbg("port_group_set: %s BUSY [%d] (%s)\n", port_groups[port_group]->obj_name, port_group, port->obj_name);
        return ENOSPC;
    }

    enet_dbg("port_group_set [%d] port %s\n", port_group, port->obj_name);
    port_groups[port_group] = port;

    return 0;
}

void port_group_unset(enetx_port_t *port)
{
    int port_group = port->port_info.port_group;

    if (port_group == -1)
        return;

    if (port_group >= PORT_GROUP_MAX)
    {
        enet_err("port_group_unset: Invalid port group [%d] (%s)\n", port_group, port->obj_name);
        return;
    }

    /* Ignore if we're not the owners */
    if (port_groups[port_group] != port)
        return;

    port_groups[port_group] = NULL;
    enet_dbg("port_group_unset [%d] freed %s\n", port_group, port->obj_name);
}
