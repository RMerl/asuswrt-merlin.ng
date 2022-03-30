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
 *  Created on: Apr/2017
 *      Author: ido.brezel@broadcom.com
 */

#ifndef _MUX_INDEX_H_
#define _MUX_INDEX_H_

#include "port.h"

/* Support for index based port mapping (de/)mux capabilities: */

/* Assign port to demux on switch at a specific index */
int mux_set_rx_index(enetx_port_t *sw, int index, enetx_port_t *port);
/* Assign port to mux on switch at a specific index */
int mux_set_tx_index(enetx_port_t *from, enetx_port_t *sw, int index, enetx_port_t *to);
/* demux function which uses sw->s.demux_map[rx_port] mapping */
int mux_get_rx_index(enetx_port_t *sw, enetx_rx_info_t *rx_info, FkBuff_t *fkb, enetx_port_t **out_port);
/* mux callback which maps to tx_port->parent_sw->parent_port */
int mux_get_tx_index(enetx_port_t *tx_port, pNBuff_t pNBuff, enetx_port_t **out_port);

void mux_index_sw_free(enetx_port_t *sw);

#endif

