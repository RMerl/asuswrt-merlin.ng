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
 *  Created on: Nov/2015
 *      Author: ido@broadcom.com
 */

#ifndef _RUNNER_H_
#define _RUNNER_H_

#include "port.h"
#include "enet.h"
#include <rdpa_api.h>
#if defined(CONFIG_BCM963146) || defined(CONFIG_BCM94912) || defined(CONFIG_BCM96813) || defined(CONFIG_BCM96855) || defined(CONFIG_BCM96888) || defined(CONFIG_BCM968880) || defined(CONFIG_BCM96837)
#include <bcm/bcmswapitypes.h>
#endif
extern int configure_bc_rate_limit_meter(void *enet_priv, unsigned int rate_limit);

static inline bdmf_object_handle _port_rdpa_object_by_port(enetx_port_t *port)
{
    if (!port)
        return NULL;

    switch (port->port_type)
    {
    case PORT_TYPE_RUNNER_PORT:
    case PORT_TYPE_G9991_PORT:
    case PORT_TYPE_RUNNER_GPON:
    case PORT_TYPE_RUNNER_EPON:
    case PORT_TYPE_SF2_PORT:
        return (bdmf_object_handle)port->priv;
    default:
        return NULL;
    }
}

#if (defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE))
// based on enet\shared\bcmenet_common.h
#define BCM_ENET_IFG        20 /* bytes */
#define BCM_ENET_CRC_LEN    4  /* bytes */
#define BCM_ENET_OVERHEAD   (BCM_ENET_CRC_LEN + BCM_ENET_IFG) /* bytes */
#endif

#if defined(CONFIG_BCM963146) || defined(CONFIG_BCM94912) || defined(CONFIG_BCM96813) || defined(CONFIG_BCM96855)
int _runner_rdpa_dos_ctrl(struct ethswctl_data *e);
#endif

#endif

