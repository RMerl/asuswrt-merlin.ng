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

