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
