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
#include "rdpa_rdd_map.h"
#include "rdpa_rdd_inline.h"
#include "rdp_drv_bbh.h"
#include "rdd_ih_defs.h"
#include "rdpa_port_int.h"
#ifndef WL4908
#include "rdd_ingress_filters.h"

rdd_vport_id_t rdpa_if_to_rdd_vport(rdpa_if port, uint8_t *rdd_wifi_ssid);

#ifndef G9991
rdd_port_profile_t rdpa_if2rdd_port_profile(rdpa_traffic_dir dir, rdpa_if port)
{
    rdd_vport_id_t rdd_vport;

    rdd_vport = rdpa_if_to_rdd_vport(port, NULL);
    return rdd_port_profile_map_get(dir, rdd_vport);
}
#endif
#endif /*!WL4908*/

/* XXX: Implement me */
rdd_vport_vector_t rdpa_ports2rdd_vport_vector(rdpa_ports ports)
{
    return 0;
}

/* XXX: Implement me */
rdpa_ports rdd_vport_vector2rdpa_ports(rdd_vport_vector_t vport_vector)
{
    return 0;
}

rdpa_ports rdpa_rdd_egress_port_vector_to_ports(uint32_t egress_port_vector, bdmf_boolean is_iptv)
{
    rdpa_ports ports = 0;
    uint32_t i, ep;
    rdpa_if port;

    for (i = RDD_EMAC_ID_START; i < RDD_EMAC_ID_COUNT; i++)
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
    rdd_emac_id_t emac;
    uint8_t wifi_ssid;

    while (ports)
    {
        rdpa_if_to_rdd_lan_mac(rdpa_port_get_next(&ports), &emac, &wifi_ssid);
        rdd_egress_port_vector |= RDD_EMAC_PORT_TO_VECTOR(emac, is_iptv);
    }
    return rdd_egress_port_vector;
}

rdd_emac_id_vector_t rdpa_if_to_rdd_emac_id_vector(rdpa_if port)
{
    rdd_emac_id_vector_t vector = 0;
    rdd_emac_id_t emac_id = RDD_EMAC_ID_COUNT;

    switch (port)
    {
#ifndef G9991
    case rdpa_if_wlan0:
    case rdpa_if_wlan1:
        emac_id = RDD_EMAC_ID_WIFI;
        break;
#endif
    case rdpa_if_lan0:
    case rdpa_if_lag0:
        emac_id = RDD_EMAC_ID_0;
        break;
    case rdpa_if_lan1:
    case rdpa_if_lag1:
        emac_id = RDD_EMAC_ID_1;
        break;
    case rdpa_if_lan2:
    case rdpa_if_lag2:
        emac_id = RDD_EMAC_ID_2;
        break;
    case rdpa_if_lan3:
    case rdpa_if_lag3:
        emac_id = RDD_EMAC_ID_3;
        break;
    case rdpa_if_lan4:
#ifndef WL4908
    case rdpa_if_lag4:
        if (!rdpa_is_gbe_mode() || _rdpa_system_init_cfg_get()->gbe_wan_emac != rdpa_emac4)
#endif
                emac_id = RDD_EMAC_ID_4;
        break;
    default:
        BDMF_TRACE_ERR("Can't convert rdpa_if %d to rdd_emac_id_vector_t\n", port);
        break;
    }

    if (emac_id != RDD_EMAC_ID_COUNT)
        vector = rdd_emac_id_to_vector(emac_id, 0);
    return vector;
}

static rdpa_ports rdpa_emac_id2to_rdpa_port_id(rdd_emac_id_t emac_id)
{
    rdpa_ports port = 0;

    switch (emac_id)
    {
#ifndef G9991
    case RDD_EMAC_ID_WIFI:
        port = rdpa_if_id(rdpa_if_wlan0);
        break;
#endif
    case RDD_EMAC_ID_0:
        port = rdpa_if_id(rdpa_if_lan0);
        break;
    case RDD_EMAC_ID_1:
        port = rdpa_if_id(rdpa_if_lan1);
        break;
    case RDD_EMAC_ID_2:
        port = rdpa_if_id(rdpa_if_lan2);
        break;
    case RDD_EMAC_ID_3:
        port = rdpa_if_id(rdpa_if_lan3);
        break;
    case RDD_EMAC_ID_4:
#ifndef WL4908
            if (!rdpa_is_gbe_mode() || _rdpa_system_init_cfg_get()->gbe_wan_emac != rdpa_emac4)
#endif
                port = rdpa_if_id(rdpa_if_lan4);
        break;
    default:
        break; /* ignore unknown bits */
    }

    return port;
}

rdd_emac_id_vector_t rdpa_ports2rdd_emac_id_vector(rdpa_ports ports)
{
    rdd_emac_id_vector_t rdd_vector = 0;
    rdpa_if port;

    while (ports)
    {
        port = rdpa_port_get_next(&ports);
        rdd_vector |= rdpa_if_to_rdd_emac_id_vector(port);
    }
    return rdd_vector;
}

rdpa_ports rdpa_rdd_emac_id_vector2rdpa_ports(rdd_emac_id_vector_t rdd_vector)
{
    rdpa_ports ports = 0;
    int i;

    for (i = 0; i < 32 && rdd_vector; i++)
    {
        int bit = (1 << i);
        if ((rdd_vector & bit))
        {
            ports |= rdpa_emac_id2to_rdpa_port_id(i);
            rdd_vector &= ~bit;
        }
    }
    return ports;
}

#if defined(WL4908)
static int2int_map_t rdpa_emac_to_bbh_emac[] =
{
    {rdpa_emac0, DRV_BBH_EMAC_0},
    {rdpa_emac1, DRV_BBH_EMAC_1},
    {rdpa_emac2, DRV_BBH_EMAC_2},
    {rdpa_emac3, DRV_BBH_WAN},
    {BDMF_ERR_PARM, BDMF_ERR_PARM},
};
#else
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
#endif

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
    {rdpa_emac0, RDD_EMAC_ID_0},
    {rdpa_emac1, RDD_EMAC_ID_1},
    {rdpa_emac2, RDD_EMAC_ID_2},
    {rdpa_emac3, RDD_EMAC_ID_3},
#if !defined(WL4908)
    {rdpa_emac4, RDD_EMAC_ID_4},
#endif
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

const int rdpa_fwd_act2rdd_fc_fwd_act[] =
{
    [rdpa_forward_action_host] = RDD_FC_FWD_ACTION_CPU,
    [rdpa_forward_action_drop] = RDD_FC_FWD_ACTION_DROP
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
    {RDPA_FILTER_ETYPE_UDEF_0, RDD_ETHER_TYPE_FILTER_USER_0},
    {RDPA_FILTER_ETYPE_UDEF_1, RDD_ETHER_TYPE_FILTER_USER_1},
    {RDPA_FILTER_ETYPE_UDEF_2, RDD_ETHER_TYPE_FILTER_USER_2},
    {RDPA_FILTER_ETYPE_UDEF_3, RDD_ETHER_TYPE_FILTER_USER_3},
    {RDPA_FILTER_ETYPE_PPPOE_D, RDD_ETHER_TYPE_FILTER_PPPOE_D},
    {RDPA_FILTER_ETYPE_PPPOE_S, RDD_ETHER_TYPE_FILTER_PPPOE_S},
    {RDPA_FILTER_ETYPE_ARP, RDD_ETHER_TYPE_FILTER_ARP},
    {RDPA_FILTER_ETYPE_PTP_1588, RDD_ETHER_TYPE_FILTER_1588},
    {RDPA_FILTER_ETYPE_802_1X, RDD_ETHER_TYPE_FILTER_802_1X},
    {RDPA_FILTER_ETYPE_802_1AG_CFM, RDD_ETHER_TYPE_FILTER_802_1AG_CFM},
    {BDMF_ERR_PARM, BDMF_ERR_PARM},
};

rdd_ether_type_filter_t rdpa_filter2rdd_etype_filter(rdpa_filter filter)
{
    return int2int_map(rdpa_filter_to_rdd_etype_flt, filter, BDMF_ERR_PARM);
}

/* rdpa_if_lan to BBH_EMAC is not applicable when LAN ports are on external switch
 * as is the case for 63138  - no need to extend to cover 7 LAN ports */
#if !defined(WL4908)
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
#else
static int2int_map_t rdpa_if_to_bbh_emac_lan[] =
{
    {rdpa_if_lan0, DRV_BBH_EMAC_0},
    {rdpa_if_lan1, DRV_BBH_EMAC_1},
    {rdpa_if_lan2, DRV_BBH_EMAC_2},
    {rdpa_if_lan3, DRV_BBH_EMAC_2},
    {rdpa_if_lan4, DRV_BBH_EMAC_2},
    {BDMF_ERR_PARM, BDMF_ERR_PARM},
};
#endif
#if defined(DSL_63138) || defined(DSL_63148)
static int2int_map_t rdpa_if_to_bbh_wan[] =
{
    {rdpa_if_wan0, DRV_BBH_EMAC_0},
    {rdpa_if_wan1, DRV_BBH_DSL},
};
#elif defined(WL4908)
static int2int_map_t rdpa_if_to_bbh_wan[] =
{
    {rdpa_if_wan0, DRV_BBH_WAN},
};
#endif

#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
int rdpa_if_to_bbh_emac(rdpa_if port)
{
    if (rdpa_if_is_lan(port))
        return int2int_map(rdpa_if_to_bbh_emac_lan, port, BDMF_ERR_PARM);

    if (rdpa_if_is_wan(port))
        return int2int_map(rdpa_if_to_bbh_wan, port, BDMF_ERR_PARM);

    return BDMF_ERR_PARM;
}
#else
int rdpa_if_to_bbh_emac(rdpa_if port)
{
    if (rdpa_if_is_lan(port))
        return int2int_map(rdpa_if_to_bbh_emac_lan, port, BDMF_ERR_PARM);

    switch (rdpa_wan_if_to_wan_type(port))
    {
#ifndef BDMF_SYSTEM_SIM
#if defined(__OREN__)
    case rdpa_wan_epon:
        return DRV_BBH_EPON;
    case rdpa_wan_gpon:
        return DRV_BBH_GPON;
#endif
#endif
    case rdpa_wan_gbe:
        return rdpa_emac2bbh_emac(_rdpa_system_init_cfg_get()->gbe_wan_emac);
    default:
        BDMF_TRACE_ERR("Can't convert rdpa_if %d to DRV_BBH_PORT_INDEX\n", port);
        break;
    }
    return BDMF_ERR_PARM;
}
#endif /*DSLWL*/

#if defined(WL4908)
/*Mapping all LAN interface to ETH0 IH class*/
static int2int_map_t rdpa_port_to_ih_class[] =
{
    {rdpa_if_lan0, DRV_RDD_IH_CLASS_LAN_BRIDGED_ETH0_INDEX},
    {rdpa_if_lan1, DRV_RDD_IH_CLASS_LAN_BRIDGED_ETH0_INDEX},
    {rdpa_if_lan2, DRV_RDD_IH_CLASS_LAN_BRIDGED_ETH0_INDEX},
    {rdpa_if_lan3, DRV_RDD_IH_CLASS_LAN_BRIDGED_ETH0_INDEX},
    {rdpa_if_lan4, DRV_RDD_IH_CLASS_LAN_BRIDGED_ETH0_INDEX},
    {rdpa_if_wan0, DRV_RDD_IH_CLASS_WAN_INDEX},
    {rdpa_if_lag0, DRV_RDD_IH_CLASS_LAN_BRIDGED_ETH0_INDEX},
    {rdpa_if_lag1, DRV_RDD_IH_CLASS_LAN_BRIDGED_ETH1_INDEX},
    {rdpa_if_lag2, DRV_RDD_IH_CLASS_LAN_BRIDGED_ETH2_INDEX},
    {BDMF_ERR_PARM, BDMF_ERR_PARM}
};
#else
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
#endif

int rdpa_port_to_ih_class_lookup(rdpa_if port)
{
    return int2int_map(rdpa_port_to_ih_class, port, BDMF_ERR_PARM);
}
