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
#include "enet_dbg.h"

static int port_vlan_port_init(enetx_port_t *self)
{
    if (mux_set_rx_index(self->p.parent_sw, self->p.port_id, self))
        return -1;
    
    if (mux_set_tx_index(self, self->p.parent_sw, self->p.port_id, self->p.parent_sw->s.parent_port))
        return -1;

    return 0;
}

static int port_vlan_port_uninit(enetx_port_t *self)
{
    return 0;
}

static int port_vlan_sw_init(enetx_port_t *self)
{
    return 0;
}

static int port_vlan_sw_uninit(enetx_port_t *self)
{
    return 0;
}

static int port_vlan_sw_port_id_on_sw(port_info_t *port_info, int *port_id, port_type_t *port_type)
{
    *port_id = port_info->port;
    *port_type = PORT_TYPE_VLAN_PORT;

    return 0;
}

static int port_vlan_sw_mux(enetx_port_t *tx_port, pNBuff_t pNBuff, enetx_port_t **out_port)
{
    /* XXX: push vlan tag */
    
    *out_port = tx_port->p.parent_sw->s.parent_port;
    enet_dbg_tx("muxed vlan_id %s on %s:%d to %s\n", tx_port->obj_name, tx_port->p.parent_sw->obj_name, tx_port->p.port_id, (*out_port) ? (*out_port)->obj_name : "error");
    if (unlikely(!*out_port))
        return -1;

    return 0;
}

static int port_vlan_sw_demux(enetx_port_t *sw, int rx_port, FkBuff_t *fkb, enetx_port_t **out_port)
{
    int vlan_id = 0; /* XXX: get vlan_id from fkb */
    
    if (unlikely(vlan_id > sw->s.demux_count))
        return -1;

    /* XXX: strip vlan tag */

    *out_port = sw->s.demux_map[vlan_id];
    enet_dbg_rx("demux vlan_id %s:%d to %s\n", sw->obj_name, vlan_id, (*out_port) ? (*out_port)->obj_name : "error");

    return 0;
}

static void port_vlan_open(enetx_port_t *self)
{
    enetx_port_t *out_port;

    port_vlan_sw_mux(self, NULL, &out_port);
    port_open(out_port);
}

static void port_vlan_stop(enetx_port_t *self)
{
    enetx_port_t *out_port;

    port_vlan_sw_mux(self, NULL, &out_port);
    port_stop(out_port);
}

sw_ops_t port_vlan_sw =
{
    .init = port_vlan_sw_init,
    .uninit = port_vlan_sw_uninit,
    .mux_port_rx = port_vlan_sw_demux,
    .mux_port_tx = port_vlan_sw_mux,
    .port_id_on_sw = port_vlan_sw_port_id_on_sw,
};

port_ops_t port_vlan_port =
{
    .init = port_vlan_port_init,
    .uninit = port_vlan_port_uninit,
    .open = port_vlan_open,
    .stop = port_vlan_stop,
};

