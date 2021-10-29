/*
   <:copyright-BRCM:2018:DUAL/GPL:standard
   
      Copyright (c) 2018 Broadcom 
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
 *  Created on: May/2017
 *      Author: steven.hsieh@broadcom.com
 */

#ifndef _SYSPVSW_H_
#define _SYSPVSW_H_

#include "port.h"

int port_sysp_port_init(enetx_port_t *self);
void port_sysp_port_open(enetx_port_t *self);
int port_sysp_mib_dump(enetx_port_t *self, int all);
int port_sysp_mib_dump_us(enetx_port_t *self, void *e); // add by Andrew
int port_sysp_port_role_set(enetx_port_t *self, port_netdev_role_t role);
int port_sysp_mtu_set(enetx_port_t *self, int mtu);

int port_sysp_sw_init(enetx_port_t *self);
int port_sysp_sw_uninit(enetx_port_t *self);

int enetxapi_post_sysp_config(void);

#endif

