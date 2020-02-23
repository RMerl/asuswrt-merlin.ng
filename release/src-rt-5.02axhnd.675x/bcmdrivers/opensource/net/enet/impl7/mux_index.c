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
 *  Created on: Apr/2017
 *      Author: ido.brezel@broadcom.com
 */

#include "mux_index.h"

static int mux_set_index(char *sw_name, enetx_port_t ***map, int *count, int index, enetx_port_t *port)
{
    int old_count = *count;

    if (!port)
    {
        if (index >= old_count)
            return -1;
    }
    else if (index >= old_count)
    {
        enetx_port_t **ports = krealloc(*map, sizeof(void *)*(index + 1), GFP_KERNEL);
        if (!ports)
        {
            enet_err("cannot de/mux %s: %s:%d not enough memory\n", port->obj_name, sw_name, index);
            return -ENOMEM;
        }

        memset(&(ports[old_count]), 0, sizeof(void *)*(index - old_count));
        *count = index + 1;
        *map = ports;
    }
    else if ((*map)[index])
    {
        enet_err("cannot de/mux %s: %s:%d already taken by %s\n", port->obj_name, sw_name, index, ((*map)[index])->obj_name);
        return -EBUSY;
    }

    rcu_assign_pointer((*map)[index], port);
    
    return 0;
}

/* demux port on switch */
int mux_set_rx_index(enetx_port_t *sw, int index, enetx_port_t *port)
{
    int rc;

    if (!(rc = mux_set_index(sw->obj_name, &sw->s.demux_map, &sw->s.demux_count, index, port)))
    {
        if (!port)
        {
            enet_dbg("demux clear: %s:%d\n", sw->obj_name, index);
            return 0;
        }

        enet_dbg("demux %s: %s:%d\n", port->obj_name, sw->obj_name, index);
    }

    return rc;
}

/* mux port on switch */
/* Usually for non-root_sw, should call mux_set_tx_index with port->p.port_id for muxing */
int mux_set_tx_index(enetx_port_t *from, enetx_port_t *sw, int index, enetx_port_t *to)
{
    int rc;

    if (!(rc = mux_set_index(sw->obj_name, &sw->s.mux_map, &sw->s.mux_count, index, to)))
    {
        if (!to)
        {
            enet_dbg("mux clear %s -> NULL: %s:%d\n", from->obj_name, sw->obj_name, index);
            return 0;
        }

        /* Cache mux OP in port object */
        from->p.mux_port_tx = sw->s.ops->mux_port_tx;
        enet_dbg("mux %s -> %s: %s:%d\n", from->obj_name, to->obj_name, sw->obj_name, index);
    }

    return rc;
}

/* This function must be called under rcu_read_lock() */
int mux_get_rx_index(enetx_port_t *sw, enetx_rx_info_t *rx_info, FkBuff_t *fkb, enetx_port_t **out_port)
{
    if (unlikely(rx_info->src_port >= sw->s.demux_count))
    {
        *out_port = NULL;
        return -1;
    }

    *out_port = rcu_dereference(sw->s.demux_map[rx_info->src_port]);
    enet_dbg_rx("demux rx_port %d tx_port %d sw: %s port: %px\n", rx_info->src_port, *out_port ? (*out_port)->p.port_id : -1, sw->obj_name, sw->s.demux_map[rx_info->src_port]);
    
#ifdef NEXT_LEVEL_DEMUX_REQUIRED
    if (unlikely(!*out_port))
        return -1;

    {
        enetx_rx_info_t rx_info2 = { .src_port = (*out_port)->p.port_id };

        sw = (*out_port)->p.child_sw;
        if (sw && sw->s.ops->mux_port_rx)
            return sw->s.ops->mux_port_rx(sw, &rx_info2, fkb, out_port);
    }
#endif

    return 0;
}

/* This function must be called under rcu_read_lock() */
int mux_get_tx_index(enetx_port_t *tx_port, pNBuff_t pNBuff, enetx_port_t **out_port)
{
    enetx_port_t *sw = tx_port->p.parent_sw;
    *out_port = rcu_dereference(sw->s.mux_map[tx_port->p.port_id]);

    enet_dbg_tx("muxed %s on %s:%d to %s\n", tx_port->obj_name, sw->obj_name, tx_port->p.port_id, (*out_port) ? (*out_port)->obj_name : "error");
    
    if (unlikely(!(*out_port)))
        return -1;

    return 0;
}

void mux_index_sw_free(enetx_port_t *sw)
{
    kfree(sw->s.demux_map);
    sw->s.demux_map = NULL;
    kfree(sw->s.mux_map);
    sw->s.mux_map = NULL;
}

