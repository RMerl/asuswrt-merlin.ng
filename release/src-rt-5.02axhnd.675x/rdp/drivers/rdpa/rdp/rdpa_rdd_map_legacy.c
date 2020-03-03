/*
* <:copyright-BRCM:2013-2015:proprietary:standard
* 
*    Copyright (c) 2013-2015 Broadcom 
*    All Rights Reserved
* 
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
* 
*  Except as expressly set forth in the Authorized License,
* 
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
* 
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
* 
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
 :>
*/

#include <bdmf_dev.h>
#include "rdpa_api.h"
#include "rdpa_common.h"
#include "rdpa_int.h"
#include "rdp_drv_bpm.h"
#include "rdp_drv_bbh.h"
#include "rdd_ih_defs.h"
#include "rdpa_rdd_inline.h"

BL_LILAC_RDD_BRIDGE_PORT_DTE rdpa_if_to_rdd_bridge_mcast_port(rdpa_if port)
{
    if (port == rdpa_if_wlan0 || port == rdpa_if_wlan1 || (port >= rdpa_if_ssid0 && port <= rdpa_if_ssid15))
        return BL_LILAC_RDD_MULTICAST_PCI_BRIDGE_PORT;
    if (port >= rdpa_if_lan0 && port <= rdpa_if_lan4)
        return BL_LILAC_RDD_MULTICAST_LAN0_BRIDGE_PORT << (port - rdpa_if_lan0); 

    BDMF_TRACE_ERR("Can't map rdpa_if %d to rdd bridge port\n", port);
    return 0;
}

BL_LILAC_RDD_BRIDGE_PORT_DTE rdpa_ports2rdd_bridge_mcast_ports_mask(rdpa_ports ports)
{
    BL_LILAC_RDD_BRIDGE_PORT_DTE rdd_bridge_ports = 0;
    rdpa_if port;

    while (ports)
    {
        port = rdpa_port_get_next(&ports);
        rdd_bridge_ports |= rdpa_if_to_rdd_bridge_mcast_port(port);
    }
    return rdd_bridge_ports;
}

rdpa_ports rdd_bridge_mcast_ports2rdpa_ports(BL_LILAC_RDD_BRIDGE_PORT_DTE rdd_bridge_mcast_ports)
{
    rdpa_ports ports = 0;
    BL_LILAC_RDD_BRIDGE_PORT_DTE bp;

    for (bp = BL_LILAC_RDD_MULTICAST_LAN0_BRIDGE_PORT;
        rdd_bridge_mcast_ports && bp <= BL_LILAC_RDD_MULTICAST_PCI_BRIDGE_PORT; bp <<= 1)
    {
        if ((rdd_bridge_mcast_ports & bp))
        {
            rdd_bridge_mcast_ports &= ~bp;
            ports |= rdpa_if_id(rdpa_rdd_bridge_port_to_if(bp, RDPA_WIFI_SSID_INVALID));
        }
    }
    return ports;
}

rdpa_ports rdpa_rdd_egress_port_vector_to_ports(uint32_t egress_port_vector, bdmf_boolean is_iptv)
{
    rdpa_ports ports = 0;
    uint32_t i, ep;
    rdpa_if port;

    for (i = BL_LILAC_RDD_EMAC_ID_START; i < BL_LILAC_RDD_EMAC_ID_COUNT; i++)
    {
        ep = RDD_EMAC_PORT_TO_VECTOR(i, is_iptv);
        if ((egress_port_vector & ep))
        {
            egress_port_vector &= ~ep;
            port = rdd_lan_mac_to_rdpa_if(i); 
            if (port != rdpa_if_none)
                ports |= rdpa_if_id(port);
        }
    }
    return ports;
}

uint32_t rdpa_ports_to_rdd_egress_port_vector(rdpa_ports ports, bdmf_boolean is_iptv)
{
    uint32_t rdd_egress_port_vector = 0;
    BL_LILAC_RDD_EMAC_ID_DTE emac;
    uint8_t wifi_ssid;

    while (ports)
    {
        rdpa_if_to_rdd_lan_mac(rdpa_port_get_next(&ports), &emac, &wifi_ssid);
        rdd_egress_port_vector |= RDD_EMAC_PORT_TO_VECTOR(emac, is_iptv);
    }
    return rdd_egress_port_vector;
}

rdpa_ports rdpa_rdd_bridge_port_mask_to_ports(BL_LILAC_RDD_BRIDGE_PORT_DTE bridge_port, uint8_t wifi_ssid)
{
    /* Single port ? */
    if (bridge_port < BL_LILAC_RDD_MULTICAST_LAN0_BRIDGE_PORT)
        return rdpa_if_id(rdpa_rdd_bridge_port_to_if(bridge_port, wifi_ssid));

    /* Mcast set */
    return rdd_bridge_mcast_ports2rdpa_ports(bridge_port);
}

BL_LILAC_RDD_BRIDGE_PORT_DTE rdpa_ports_to_rdd_bridge_port_mask(rdpa_ports ports, uint8_t *wifi_ssid)
{
    if (rdpa_port_is_single(ports))
        return rdpa_if_to_rdd_bridge_port(rdpa_port_get_next(&ports), wifi_ssid);

    return rdpa_ports2rdd_bridge_mcast_ports_mask(ports);
}

BL_LILAC_RDD_BRIDGE_PORT_VECTOR_DTE rdpa_if_to_rdd_bridge_port_vector(rdpa_if port)
{
    BL_LILAC_RDD_BRIDGE_PORT_VECTOR_DTE vector = 0;

    switch (port)
    {
    case rdpa_if_wlan0:
    case rdpa_if_wlan1:
        vector = BL_LILAC_RDD_BRIDGE_PORT_VECTOR_PCI;
        break;
    case rdpa_if_lan0:
    case rdpa_if_lag0:
        vector = BL_LILAC_RDD_BRIDGE_PORT_VECTOR_LAN0;
        break;
    case rdpa_if_lan1:
    case rdpa_if_lag1:
        vector = BL_LILAC_RDD_BRIDGE_PORT_VECTOR_LAN1;
        break;
    case rdpa_if_lan2:
    case rdpa_if_lag2:
        vector = BL_LILAC_RDD_BRIDGE_PORT_VECTOR_LAN2;
        break;
    case rdpa_if_lan3:
    case rdpa_if_lag3:
        vector = BL_LILAC_RDD_BRIDGE_PORT_VECTOR_LAN3;
        break;
    case rdpa_if_lan4:
    case rdpa_if_lag4:
        {
            if (!rdpa_is_gbe_mode() || _rdpa_system_init_cfg_get()->gbe_wan_emac != rdpa_emac4)
                vector = BL_LILAC_RDD_BRIDGE_PORT_VECTOR_LAN4;
        }
        break;

    default:
        BDMF_TRACE_ERR("Can't convert rdpa_if %d to BL_LILAC_RDD_BRIDGE_PORT_VECTOR_DTE\n", port);
        break;
    }
    return vector;
}

static rdpa_ports rdpa_rdd_bridge_port_vector_to_rdpa_port(BL_LILAC_RDD_BRIDGE_PORT_VECTOR_DTE vector)
{
    rdpa_ports port = 0;

    switch (vector)
    {
    case BL_LILAC_RDD_BRIDGE_PORT_VECTOR_PCI:
        port = rdpa_if_id(rdpa_if_wlan0);
        break;
    case BL_LILAC_RDD_BRIDGE_PORT_VECTOR_LAN0:
        port = rdpa_if_id(rdpa_if_lan0);
        break;
    case BL_LILAC_RDD_BRIDGE_PORT_VECTOR_LAN1:
        port = rdpa_if_id(rdpa_if_lan1);
        break;
    case BL_LILAC_RDD_BRIDGE_PORT_VECTOR_LAN2:
        port = rdpa_if_id(rdpa_if_lan2);
        break;
    case BL_LILAC_RDD_BRIDGE_PORT_VECTOR_LAN3:
        port = rdpa_if_id(rdpa_if_lan3);
        break;
    case BL_LILAC_RDD_BRIDGE_PORT_VECTOR_LAN4:
        {
            if (!rdpa_is_gbe_mode() || _rdpa_system_init_cfg_get()->gbe_wan_emac != rdpa_emac4)
                port = rdpa_if_id(rdpa_if_lan4);
        }
        break;
    default:
        break; /* ignore unknown bits */
    }

    return port;
}

BL_LILAC_RDD_BRIDGE_PORT_VECTOR_DTE rdpa_ports2rdd_bridge_port_vector(rdpa_ports ports)
{
    BL_LILAC_RDD_BRIDGE_PORT_VECTOR_DTE rdd_vector = 0;
    rdpa_if port;

    while (ports)
    {
        port = rdpa_port_get_next(&ports);
        rdd_vector |= rdpa_if_to_rdd_bridge_port_vector(port);
    }
    return rdd_vector;
}

rdpa_ports rdpa_rdd_bridge_port_vector2rdpa_ports(BL_LILAC_RDD_BRIDGE_PORT_VECTOR_DTE rdd_vector)
{
    rdpa_ports ports = 0;
    int i;

    for (i = 0; i < 32 && rdd_vector; i++)
    {
        int bit = (1 << i);
        if ((rdd_vector & bit))
        {
            ports |= rdpa_rdd_bridge_port_vector_to_rdpa_port(bit);
            rdd_vector &= ~bit;
        }
    }
    return ports;
}

/* Wrapper mapping functions to convert rdpa_wan_type to
 * egress_phy type. */
static int2int_map_t rdpa_wan_type_to_rdd_egress_phy[] =
{
#if defined(DSL_63138) || defined(DSL_63148)
    {rdpa_wan_gbe, rdd_egress_phy_eth_wan},
    {rdpa_wan_dsl, rdd_egress_phy_dsl},
#endif
    {BDMF_ERR_PARM, BDMF_ERR_PARM},
};

int rdpa_wan_type2rdd_egress_phy(rdpa_wan_type src)
{
    return int2int_map(rdpa_wan_type_to_rdd_egress_phy, src, BDMF_ERR_PARM);
}
rdpa_wan_type rdd_egress_phy2rdpa_wan_type(int src)
{
    return int2int_map_r(rdpa_wan_type_to_rdd_egress_phy, src, BDMF_ERR_PARM);
}

static int2int_map_t rdpa_wan_emac_to_rdd_port[] =
{
#if defined(DSL_63138) || defined(DSL_63148)
    {rdpa_emac0, BL_LILAC_RDD_WAN_PHYSICAL_PORT_ETH0},
#endif
    {rdpa_emac4, BL_LILAC_RDD_WAN_PHYSICAL_PORT_ETH4},
#if defined(DSL_63138) || defined(DSL_63148)
    {rdpa_emac5, BL_LILAC_RDD_WAN_PHYSICAL_PORT_ETH5},
#else
    {rdpa_emac5, BL_LILAC_RDD_WAN_PHYSICAL_PORT_FIBER},
#endif
    {rdpa_emac0, BL_LILAC_RDD_WAN_PHYSICAL_PORT_ETH0},
#if defined(__OREN__)
    {rdpa_emac1, BL_LILAC_RDD_WAN_PHYSICAL_PORT_ETH1},
    {rdpa_emac2, BL_LILAC_RDD_WAN_PHYSICAL_PORT_ETH2},
    {rdpa_emac3, BL_LILAC_RDD_WAN_PHYSICAL_PORT_ETH3},
#endif

    {BDMF_ERR_PARM, BDMF_ERR_PARM},
};

int rdpa_wan_emac2rdd_phys_port(rdpa_emac_mode src)
{
    return int2int_map(rdpa_wan_emac_to_rdd_port, src, BDMF_ERR_PARM);
}

static int2int_map_t rdpa_emac_to_bpm_dp_emac[] =
{
    {rdpa_emac0, DRV_BPM_SP_EMAC0},
    {rdpa_emac1, DRV_BPM_SP_EMAC1},
    {rdpa_emac2, DRV_BPM_SP_EMAC2},
    {rdpa_emac3, DRV_BPM_SP_EMAC3},
    {rdpa_emac4, DRV_BPM_SP_EMAC4},
    {rdpa_emac5, DRV_BPM_SP_GPON},
    {BDMF_ERR_PARM, BDMF_ERR_PARM},
};

int rdpa_emac2bpm_emac(rdpa_emac src)
{
    return int2int_map(rdpa_emac_to_bpm_dp_emac, src, BDMF_ERR_PARM);
}

static int2int_map_t rdpa_emac_to_bbh_emac[] =
{
    {rdpa_emac0, DRV_BBH_EMAC_0},
    {rdpa_emac1, DRV_BBH_EMAC_1},
    {rdpa_emac2, DRV_BBH_EMAC_2},
    {rdpa_emac3, DRV_BBH_EMAC_3},
    {rdpa_emac4, DRV_BBH_EMAC_4},
    {rdpa_emac5, DRV_BBH_EMAC_5},
    {BDMF_ERR_PARM, BDMF_ERR_PARM},
};

int rdpa_emac2bbh_emac(rdpa_emac src)
{
    return int2int_map(rdpa_emac_to_bbh_emac, src, BDMF_ERR_PARM);
}

rdpa_emac bbh_emac2_emac(int src)
{
    return rdpa_emac0 + ((DRV_BBH_PORT_INDEX)src - DRV_BBH_EMAC_0);
}

static int2int_map_t rdpa_emac_to_rdd_emac[] =
{
    {rdpa_emac0, BL_LILAC_RDD_EMAC_ID_0},
    {rdpa_emac1, BL_LILAC_RDD_EMAC_ID_1},
    {rdpa_emac2, BL_LILAC_RDD_EMAC_ID_2},
    {rdpa_emac3, BL_LILAC_RDD_EMAC_ID_3},
    {rdpa_emac4, BL_LILAC_RDD_EMAC_ID_4},
#if defined(DSL_63138) || defined(DSL_63148)
    /* For 63138 no need to extend this mapping
     * Only emac0 (ETHWAN), emac1 (ETHLAN to SF2) and emac2 (DSL)
     * are used. */
#endif
    {BDMF_ERR_PARM, BDMF_ERR_PARM},
};

int rdpa_emac2rdd_emac(rdpa_emac src)
{
    return int2int_map(rdpa_emac_to_rdd_emac, src, BDMF_ERR_PARM);
}

static int2int_map_t rdpa_emac_to_eth_thread[] =
{
#if defined(DSL_63138) || defined(DSL_63148)
    /* Intentionally not extending this mapping for 63138 
     * Only emac0 (ETHWAN), emac1 (ETHLAN to SF2) and emac2 (DSL)
     * are used. */
    {rdpa_emac0, WAN1_TX_THREAD_NUMBER},
    {rdpa_emac1, ETH_TX_THREAD_NUMBER},
    {rdpa_emac2, WAN_TX_THREAD_NUMBER},
#elif defined(WL4908)
    {rdpa_emac0, ETH_TX_THREAD_NUMBER},
    {rdpa_emac1, ETH_TX_THREAD_NUMBER},
    {rdpa_emac2, ETH_TX_THREAD_NUMBER},
    {rdpa_emac3, WAN1_TX_THREAD_NUMBER},
#else
    {rdpa_emac0, ETH0_TX_THREAD_NUMBER},
    {rdpa_emac1, ETH1_TX_THREAD_NUMBER},
    {rdpa_emac2, ETH2_TX_THREAD_NUMBER},
    {rdpa_emac3, ETH3_TX_THREAD_NUMBER},
#endif
    {BDMF_ERR_PARM, BDMF_ERR_PARM},
};

int rdpa_emac2rdd_eth_thread(rdpa_emac src)
{
    return int2int_map(rdpa_emac_to_eth_thread, src, BDMF_ERR_PARM);
}

const int rdpa_fwd_act2rdd_fc_fwd_act[] =
{
    [rdpa_forward_action_host] = RDD_FLOW_CACHE_FORWARD_ACTION_CPU,
    [rdpa_forward_action_drop] = RDD_FLOW_CACHE_FORWARD_ACTION_DROP
};

#define NOT_CPU_DIRECTED -1

static int2int_map_t rdpa_cpu_dest_to_rdd_cpu_direct_q[] =
{
    {rdpa_flow_dest_omci, rdpa_cpu_rx_reason_omci},
    {NOT_CPU_DIRECTED, 0},
};

int rdpa_dest_cpu2rdd_direct_q(rdpa_emac src)
{
    return int2int_map(rdpa_cpu_dest_to_rdd_cpu_direct_q, src, NOT_CPU_DIRECTED);
}

static int2int_map_t rdpa_filter_to_rdd_etype_flt[] =
{
    {RDPA_FILTER_ETYPE_UDEF_0, BL_LILAC_RDD_ETHER_TYPE_FILTER_USER_0},
    {RDPA_FILTER_ETYPE_UDEF_1, BL_LILAC_RDD_ETHER_TYPE_FILTER_USER_1},
    {RDPA_FILTER_ETYPE_UDEF_2, BL_LILAC_RDD_ETHER_TYPE_FILTER_USER_2},
    {RDPA_FILTER_ETYPE_UDEF_3, BL_LILAC_RDD_ETHER_TYPE_FILTER_USER_3},
    {RDPA_FILTER_ETYPE_PPPOE_D, BL_LILAC_RDD_ETHER_TYPE_FILTER_PPPOE_D},
    {RDPA_FILTER_ETYPE_PPPOE_S, BL_LILAC_RDD_ETHER_TYPE_FILTER_PPPOE_S},
    {RDPA_FILTER_ETYPE_ARP, BL_LILAC_RDD_ETHER_TYPE_FILTER_ARP},
    {RDPA_FILTER_ETYPE_PTP_1588, BL_LILAC_RDD_ETHER_TYPE_FILTER_1588},
    {RDPA_FILTER_ETYPE_802_1X, BL_LILAC_RDD_ETHER_TYPE_FILTER_802_1X},
    {RDPA_FILTER_ETYPE_802_1AG_CFM, BL_LILAC_RDD_ETHER_TYPE_FILTER_802_1AG_CFM},
    {BDMF_ERR_PARM, BDMF_ERR_PARM},
};

BL_LILAC_RDD_ETHER_TYPE_FILTER_NUMBER_DTE rdpa_filter_to_rdd_etype_filter(rdpa_filter filter)
{
    return int2int_map(rdpa_filter_to_rdd_etype_flt, filter, BDMF_ERR_PARM);
}
/* rdpa_if_lan to BBH_EMAC is not applicable when LAN ports are on external switch 
 * as is the case for 63138  - no need to extend to cover 7 LAN ports */
static int2int_map_t rdpa_if_to_bbh_emac_lan[] = 
{
    {rdpa_if_lan0, DRV_BBH_EMAC_0},
    {rdpa_if_lan1, DRV_BBH_EMAC_1},
    {rdpa_if_lan2, DRV_BBH_EMAC_2},
    {rdpa_if_lan3, DRV_BBH_EMAC_3},
    {rdpa_if_lan4, DRV_BBH_EMAC_4},
    {rdpa_if_lan5, DRV_BBH_EMAC_5},
    {BDMF_ERR_PARM, BDMF_ERR_PARM},
};
#if defined(DSL_63138) || defined(DSL_63148)
static int2int_map_t rdpa_if_to_bbh_wan[] =
{
    {rdpa_if_wan0, DRV_BBH_EMAC_0},
    {rdpa_if_wan1, DRV_BBH_DSL},
    {BDMF_ERR_PARM, BDMF_ERR_PARM},
};
#endif
int rdpa_if_to_bbh_emac(rdpa_if port)
{
    if (rdpa_if_is_lan(port))
        return int2int_map(rdpa_if_to_bbh_emac_lan, port, BDMF_ERR_PARM);

#if defined(DSL_63138) || defined(DSL_63148)
    if (rdpa_if_is_wan(port))
        return int2int_map(rdpa_if_to_bbh_wan, port, BDMF_ERR_PARM);
#else
    switch (rdpa_wan_if_to_wan_type(port))
    {
    case rdpa_wan_gpon:
        return DRV_BBH_GPON;
    case rdpa_wan_gbe:
        return rdpa_emac2bbh_emac(_rdpa_system_init_cfg_get()->gbe_wan_emac);
#ifndef BDMF_SYSTEM_SIM
#if defined(__OREN__)
    case rdpa_wan_epon:
        return DRV_BBH_EPON;
#endif
#endif
    default:
        BDMF_TRACE_ERR("Can't convert rdpa_if %d to DRV_BBH_PORT_INDEX\n", port);
        break;
    }
#endif
    return BDMF_ERR_PARM;
}

static int2int_map_t rdpa_port_to_ih_class[] =
{
    {rdpa_if_lan0, DRV_RDD_IH_CLASS_LAN_BRIDGED_ETH0_INDEX},
    {rdpa_if_lan1, DRV_RDD_IH_CLASS_LAN_BRIDGED_ETH1_INDEX},
    {rdpa_if_lan2, DRV_RDD_IH_CLASS_LAN_BRIDGED_ETH2_INDEX},
    {rdpa_if_lan3, DRV_RDD_IH_CLASS_LAN_BRIDGED_ETH3_INDEX},
    {rdpa_if_lan4, DRV_RDD_IH_CLASS_LAN_BRIDGED_ETH4_INDEX},
    {rdpa_if_wan0, DRV_RDD_IH_CLASS_WAN_BRIDGED_HIGH_INDEX},
    {rdpa_if_lag0, DRV_RDD_IH_CLASS_LAN_BRIDGED_ETH0_INDEX},
    {rdpa_if_lag1, DRV_RDD_IH_CLASS_LAN_BRIDGED_ETH1_INDEX},
    {rdpa_if_lag2, DRV_RDD_IH_CLASS_LAN_BRIDGED_ETH2_INDEX},
    {rdpa_if_lag3, DRV_RDD_IH_CLASS_LAN_BRIDGED_ETH3_INDEX},
    {rdpa_if_lag4, DRV_RDD_IH_CLASS_LAN_BRIDGED_ETH4_INDEX},
    {BDMF_ERR_PARM, BDMF_ERR_PARM}
};

int rdpa_port_to_ih_class_lookup(rdpa_if port)
{
    return int2int_map(rdpa_port_to_ih_class, port, BDMF_ERR_PARM);
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

int emac_id2rdd_bridge(rdpa_emac src)
{
    return int2int_map(emac_to_rdd_bridge_port, src, BDMF_ERR_PARM);
}

int rdd_bridge2emac_id(rdpa_emac src)
{
    return int2int_map_r(emac_to_rdd_bridge_port, src, BDMF_ERR_PARM);
}
