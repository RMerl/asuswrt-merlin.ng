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

#include "port.h"

#ifdef RUNNER
#include "runner.h"
#endif
#ifdef SF2_DEVICE
#include "sf2.h"
#endif
#ifdef CONFIG_BCM_FTTDP_G9991
#include "g9991.h"
#endif
#include "runner_wifi.h"

#ifdef VLANTAG
extern sw_ops_t port_vlan_sw;
extern port_ops_t port_vlan_port;
#endif

#ifdef ENET_DMA
extern sw_ops_t port_dummy_sw;
extern port_ops_t port_dma_port;
#endif

#ifdef RUNNER
extern sw_ops_t port_runner_sw;
extern port_ops_t port_runner_port;
extern port_ops_t port_runner_port_mac;
#ifdef EPON
extern port_ops_t port_runner_epon;
#endif
#ifdef GPON
extern port_ops_t port_runner_gpon;
#endif
#endif

#ifdef SYSPVSW_DEVICE
extern sw_ops_t port_sysp_sw;
extern port_ops_t port_sysp_port;
extern port_ops_t port_sysp_port_mac;
#endif 

static int dbg_port_count, dbg_sw_count;

int _assign_port_class(enetx_port_t *port, port_type_t port_type)
{
    sw_ops_t *sw_ops;
    port_ops_t *port_ops = NULL;
    port_class_t port_class = -1;

    switch (port_type)
    {
        case PORT_TYPE_DETECT:
            port_class = PORT_CLASS_PORT_DETECT;
            break;
#ifdef RUNNER
        case PORT_TYPE_RUNNER_SW:
            sw_ops = &port_runner_sw;
            port_class = PORT_CLASS_SW;
            break;
        case PORT_TYPE_RUNNER_PORT:
            port_ops = &port_runner_port;
            port_class = PORT_CLASS_PORT;
            break;
        case PORT_TYPE_RUNNER_MAC:
            port_ops = &port_runner_port_mac;
            port_class = PORT_CLASS_PORT;
            break;
#endif
#ifdef SF2_DEVICE
        case PORT_TYPE_SF2_SW:
            sw_ops = &port_sf2_sw;
            port_class = PORT_CLASS_SW;
            break;
        case PORT_TYPE_SF2_PORT:
            port_ops = &port_sf2_port;
            port_class = PORT_CLASS_PORT;
            break;
        case PORT_TYPE_SF2_MAC:
            port_ops = &port_sf2_port_mac;
            port_class = PORT_CLASS_PORT;
            break;
#endif
#ifdef SYSPVSW_DEVICE
        case PORT_TYPE_SYSP_SW:
            sw_ops = &port_sysp_sw;
            port_class = PORT_CLASS_SW;
            break;
        case PORT_TYPE_SYSP_PORT:
            port_ops = &port_sysp_port;
            port_class = PORT_CLASS_PORT;
            break;
        case PORT_TYPE_SYSP_MAC:
            port_ops = &port_sysp_port_mac;
            port_class = PORT_CLASS_PORT;
            break;
#endif
#ifdef ENET_RUNNER_WIFI
        case PORT_TYPE_RUNNER_WIFI:
            port_ops = &port_runner_wifi;
            port_class = PORT_CLASS_PORT;
            break;
#endif
#ifdef EPON
        case PORT_TYPE_RUNNER_EPON:
            port_ops = &port_runner_epon;
            port_class = PORT_CLASS_PORT;
            break;
#endif
#ifdef GPON
        case PORT_TYPE_RUNNER_GPON:
            port_ops = &port_runner_gpon;
            port_class = PORT_CLASS_PORT;
            break;
#endif
#ifdef CONFIG_BCM_FTTDP_G9991
        case PORT_TYPE_G9991_SW:
            sw_ops = &port_g9991_sw;
            port_class = PORT_CLASS_SW;
            break;
        case PORT_TYPE_G9991_PORT:
            port_ops = &port_g9991_port;
            port_class = PORT_CLASS_PORT;
            break;
        case PORT_TYPE_G9991_ES_PORT:
            port_ops = &port_g9991_es_port;
            port_class = PORT_CLASS_PORT;
            break;
#endif
#ifdef VLANTAG
        case PORT_TYPE_VLAN_SW:
            sw_ops = &port_vlan_sw;
            port_class = PORT_CLASS_SW;
            break;
        case PORT_TYPE_VLAN_PORT:
            port_ops = &port_vlan_port;
            port_class = PORT_CLASS_PORT;
            break;
#endif
#ifdef ENET_DMA
        case PORT_TYPE_DIRECT_RGMII:
            sw_ops = &port_dummy_sw;
            port_class = PORT_CLASS_SW;
            break;
        case PORT_TYPE_GENERIC_DMA:
            port_ops = &port_dma_port;
            port_class = PORT_CLASS_PORT;
            break;
#endif
        default:
            enet_err("failed to create port type %d port_class %d\n", port_type, port_class);
            return -1;
    }

    port->port_class = port_class;
    port->port_type = port_type;

    /* Object name for easy debugging */
    if (port->port_class == PORT_CLASS_SW)
    {
        port->s.ops = sw_ops;
        if (!port->obj_name[0])
            snprintf(port->obj_name, IFNAMSIZ, "sw%d", dbg_sw_count++);
    }
    else if (port->port_class == PORT_CLASS_PORT || port->port_class == PORT_CLASS_PORT_DETECT)
    {
        port->p.ops = port_ops;
        if (!port->obj_name[0])
            snprintf(port->obj_name, IFNAMSIZ, "port%d", dbg_port_count++);
    }

    enet_dbg("set %s type %d\n", port->obj_name, port_type);
            
    return 0;
}

