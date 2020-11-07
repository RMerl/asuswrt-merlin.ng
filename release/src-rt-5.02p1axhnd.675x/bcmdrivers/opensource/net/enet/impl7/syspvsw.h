/*
   <:copyright-BRCM:2018:DUAL/GPL:standard
   
      Copyright (c) 2018 Broadcom 
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

