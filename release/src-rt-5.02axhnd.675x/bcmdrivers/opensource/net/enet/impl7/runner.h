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
 *  Created on: Nov/2015
 *      Author: ido@broadcom.com
 */

#ifndef _RUNNER_H_
#define _RUNNER_H_

#include "port.h"
#include "enet.h"
#include <rdpa_api.h>

extern int configure_bc_rate_limit_meter(int port_id, unsigned int rate_limit);

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

static inline int _port_rdpa_if_by_port(enetx_port_t *port, rdpa_if *index)
{
    bdmf_object_handle port_obj = _port_rdpa_object_by_port(port);

    if (port_obj)
        return rdpa_port_index_get(port_obj, index);

    return -1;
}

#if (defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE))
// based on enet\shared\bcmenet_common.h
#define BCM_ENET_IFG        20 /* bytes */
#define BCM_ENET_CRC_LEN    4  /* bytes */
#define BCM_ENET_OVERHEAD   (BCM_ENET_CRC_LEN + BCM_ENET_IFG) /* bytes */
#endif

#endif

