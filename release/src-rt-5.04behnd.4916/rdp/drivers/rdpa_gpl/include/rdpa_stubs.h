/*
 * <:copyright-BRCM:2013:DUAL/GPL:standard
 * 
 *    Copyright (c) 2013 Broadcom 
 *    All Rights Reserved
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 * 
 * :> 
 */


#ifndef _RDPA_STUBS_H_
#define _RDPA_STUBS_H_

#include "rdpa_types.h"
#include "rdpa_filter.h"
#include "rdpa_port.h"
#include "rdpa_ingress_class_basic.h"
#include "rdpa_cpu.h"
#include "rdd_data_structures_auto.h"
#include "rdd_defs.h"

/* DEPRICATED, MUST BE REMOVED FROM THE CODE */
static inline rdpa_ports rdpa_rdd_bridge_port_mask_to_ports(rdd_bridge_port_t bridge_port, uint8_t wifi_ssid)
{
    return 0;
}

static inline rdd_bridge_port_t rdpa_ports_to_rdd_bridge_port_mask(rdpa_ports ports, uint8_t *wifi_ssid)
{
    return 0;
}

static inline rdd_bridge_port_t rdpa_if_to_rdd_bridge_mcast_port(rdpa_if port)
{
    return 0;
}

static inline rdd_bridge_port_t rdpa_ports2rdd_bridge_mcast_ports_mask(rdpa_ports ports)
{
    return 0;
}

static inline rdpa_ports rdd_bridge_mcast_ports2rdpa_ports(rdd_bridge_port_t rdd_bridge_mcast_ports)
{
    return 0;
}

static int2int_map_t emac_to_rdd_bridge_port[] =
{
    {rdpa_emac0, BL_LILAC_RDD_LAN0_BRIDGE_PORT},
    {rdpa_emac1, BL_LILAC_RDD_LAN1_BRIDGE_PORT},
    {rdpa_emac2, BL_LILAC_RDD_LAN2_BRIDGE_PORT},
    {rdpa_emac3, BL_LILAC_RDD_LAN3_BRIDGE_PORT},
    {rdpa_emac4, BL_LILAC_RDD_LAN4_BRIDGE_PORT},
    {BDMF_ERR_PARM, BDMF_ERR_PARM},
};

static inline int emac_id2rdd_bridge(rdpa_emac src)
{
    return int2int_map(emac_to_rdd_bridge_port, src, BDMF_ERR_PARM);
}

static inline int rdd_bridge2emac_id(rdpa_emac src)
{
    return int2int_map_r(emac_to_rdd_bridge_port, src, BDMF_ERR_PARM);
}

#endif
