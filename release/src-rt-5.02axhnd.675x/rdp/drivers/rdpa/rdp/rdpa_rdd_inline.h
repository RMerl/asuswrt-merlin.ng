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
* :>
*/

/*
 * RDPA RDD common functions
 */

#ifndef _RDPA_RDD_INLINE_H_
#define _RDPA_RDD_INLINE_H_

#include "rdd_defs.h"
#ifdef LEGACY_RDP
#include "rdpa_rdd_map_legacy.h"
#else
#include "rdd_runner_proj_defs.h"
#include "rdpa_rdd_map.h"
#endif
#include "rdpa_types.h"
#include "rdpa_int.h"

static inline rdpa_if rdpa_rdd_bridge_port_to_if(rdd_bridge_port_t bridge_port, uint8_t wifi_ssid)
{
    rdpa_if port = rdpa_if_none;

    switch (bridge_port)
    {
#if defined(DSL_63138) || defined(DSL_63148)
    case BL_LILAC_RDD_WAN0_BRIDGE_PORT:
        port = rdpa_wan_type_to_if(rdpa_wan_dsl);
        break;

    case BL_LILAC_RDD_WAN1_BRIDGE_PORT:
        port = rdpa_wan_type_to_if(rdpa_wan_gbe);
        break;

#else
    case BL_LILAC_RDD_WAN_BRIDGE_PORT:
#endif
    case BL_LILAC_RDD_WAN_ROUTER_PORT:
    case BL_LILAC_RDD_WAN_IPTV_BRIDGE_PORT:
        port = rdpa_if_wan0; /* FIXME: MULTI-WAN XPON */
        break;

    case BL_LILAC_RDD_LAN0_BRIDGE_PORT:
    case BL_LILAC_RDD_MULTICAST_LAN0_BRIDGE_PORT:
#if defined(BCM_DSL_RDP)
        port = rdpa_if_lan0;
#else
        port = (rdpa_is_gbe_mode() && (rdpa_gbe_wan_emac() == rdpa_emac0)) ? rdpa_wan_type_to_if(rdpa_wan_gbe) : rdpa_if_lan0; 
#endif

        break;

    case BL_LILAC_RDD_LAN1_BRIDGE_PORT:
    case BL_LILAC_RDD_MULTICAST_LAN1_BRIDGE_PORT:
        port = (rdpa_is_gbe_mode() && (rdpa_gbe_wan_emac() == rdpa_emac1)) ? rdpa_wan_type_to_if(rdpa_wan_gbe) : rdpa_if_lan1; 
        break;

    case BL_LILAC_RDD_LAN2_BRIDGE_PORT:
    case BL_LILAC_RDD_MULTICAST_LAN2_BRIDGE_PORT:
        port = (rdpa_is_gbe_mode() && (rdpa_gbe_wan_emac() == rdpa_emac2)) ? rdpa_wan_type_to_if(rdpa_wan_gbe) : rdpa_if_lan2;
        break;

    case BL_LILAC_RDD_LAN3_BRIDGE_PORT:
    case BL_LILAC_RDD_MULTICAST_LAN3_BRIDGE_PORT:
        port = (rdpa_is_gbe_mode() && (rdpa_gbe_wan_emac() == rdpa_emac3)) ? rdpa_wan_type_to_if(rdpa_wan_gbe) : rdpa_if_lan3;
        break;

    case BL_LILAC_RDD_LAN4_BRIDGE_PORT:
    case BL_LILAC_RDD_MULTICAST_LAN4_BRIDGE_PORT:
        port = (rdpa_is_gbe_mode() && (rdpa_gbe_wan_emac() == rdpa_emac4)) ? rdpa_wan_type_to_if(rdpa_wan_gbe) : rdpa_if_lan4;
        break;

#ifndef G9991
    case BL_LILAC_RDD_PCI_BRIDGE_PORT:
        port = wifi_ssid == RDPA_WIFI_SSID_INVALID ? rdpa_if_wlan0 : rdpa_if_ssid0 + wifi_ssid;
        break;
#endif

    case BL_LILAC_RDD_MULTICAST_PCI_BRIDGE_PORT:
        port = rdpa_if_wlan0;
        break;

    case BL_LILAC_RDD_CPU_BRIDGE_PORT:
        port = rdpa_if_cpu;
        break;

    case BL_LILAC_RDD_VIRTUAL_BRIDGE_PORT:
#ifdef G9991
    case BL_LILAC_RDD_G9991_BRIDGE_PORT:
#endif
        port = rdpa_if_switch;
        break;

#if defined(BCM_DSL_RDP)
    case BL_LILAC_RDD_LAN5_BRIDGE_PORT:
        port = rdpa_if_lan5;
        break;

    case BL_LILAC_RDD_LAN6_BRIDGE_PORT:
        port = rdpa_if_lan6;
        break;

    case BL_LILAC_RDD_LAN7_BRIDGE_PORT:
        port = rdpa_if_lan7;
        break;
#endif

#if defined(BCM_DSL_RDP)
    case BL_LILAC_RDD_ANY_BRIDGE_PORT:
        port = rdpa_if_any;
        break;
#endif

    default:
        BDMF_TRACE_ERR("Can't map rdd bridge port 0x%x to rdpa_if\n", bridge_port);
        port = rdpa_if_none;
        break;
    }
    return port;
}


#if defined(DSL_63138) || defined(DSL_63148)
static inline rdpa_if rdpa_rdd_bridge_port_to_enet_if(rdd_bridge_port_t bridge_port)
{
    if (bridge_port == BL_LILAC_RDD_WAN1_BRIDGE_PORT)
        return rdpa_wan_type_to_if(rdpa_wan_gbe); 
    return bridge_port - BL_LILAC_RDD_LAN0_BRIDGE_PORT + rdpa_if_lan0;
}
#elif defined(WL4908)
static inline rdpa_if rdpa_rdd_bridge_port_to_enet_if(rdd_bridge_port_t bridge_port)
{
    if (bridge_port == BL_LILAC_RDD_WAN_BRIDGE_PORT)
        return rdpa_wan_type_to_if(rdpa_wan_gbe);
    return bridge_port - BL_LILAC_RDD_LAN0_BRIDGE_PORT + rdpa_if_lan0;
}
#endif /* (DSL_63138 || DSL_63148) */

static inline rdd_bridge_port_t rdpa_if_to_rdd_bridge_port(rdpa_if port, uint8_t *rdd_wifi_ssid)
{
   rdd_bridge_port_t bridge_port;

   if (rdd_wifi_ssid)
       *rdd_wifi_ssid = 0;

    switch (port)
    {
#if defined(DSL_63138) || defined(DSL_63148)
    case rdpa_if_wan0:
        bridge_port = BL_LILAC_RDD_WAN1_BRIDGE_PORT;
        break;
    case rdpa_if_wan1:
        bridge_port = BL_LILAC_RDD_WAN0_BRIDGE_PORT;
        break;
#else
    case rdpa_if_wan0:
        bridge_port = BL_LILAC_RDD_WAN_BRIDGE_PORT;
        break;
#endif
    case rdpa_if_lan0 ... rdpa_if_lan_max:
        bridge_port = port - rdpa_if_lan0 + BL_LILAC_RDD_LAN0_BRIDGE_PORT;
        break;
    case rdpa_if_cpu:
        bridge_port = BL_LILAC_RDD_CPU_BRIDGE_PORT;
        break;
    case rdpa_if_ssid0 ... rdpa_if_ssid15:     
        if (rdd_wifi_ssid)
            *rdd_wifi_ssid = port - rdpa_if_ssid0;
    case rdpa_if_wlan0:
    case rdpa_if_wlan1:
        bridge_port = BL_LILAC_RDD_PCI_BRIDGE_PORT;
        break;
    case rdpa_if_lag0 ... rdpa_if_lag4:
        bridge_port = port - rdpa_if_lag0 + BL_LILAC_RDD_LAN0_BRIDGE_PORT;
        break;
    case rdpa_if_switch:
#ifdef G9991
        bridge_port = BL_LILAC_RDD_G9991_BRIDGE_PORT;
#else
        bridge_port = BL_LILAC_RDD_VIRTUAL_BRIDGE_PORT;
#endif
        break;
#if defined(BCM_DSL_RDP)
    case rdpa_if_any:
        bridge_port = BL_LILAC_RDD_ANY_BRIDGE_PORT;
        break;
#endif
    default:
        if (port != rdpa_if_none)
            BDMF_TRACE_ERR("Can't map rdpa_if %d to rdd bridge port\n", port);
#if defined(DSL_63138) || defined(DSL_63148)
        bridge_port = BL_LILAC_RDD_WAN0_BRIDGE_PORT;
#else
        bridge_port = BL_LILAC_RDD_WAN_BRIDGE_PORT;
#endif
        break;
    }
    return bridge_port;
}

static inline int rdpa_if_to_rdd_lan_mac(rdpa_if port, rdd_emac_id_t *rdd_emac, uint8_t *wifi_ssid)
{
    if (port >= rdpa_if_lan0 && port <= rdpa_if_lan_max)
    {
        *rdd_emac = RDD_EMAC_ID_0 + (port - rdpa_if_lan0);
        *wifi_ssid = 0;
        return 0;
    }
    if ((port >= rdpa_if_ssid0 && port <= rdpa_if_ssid15) ||
        (port >= rdpa_if_wlan0 && port <= rdpa_if_wlan1))
    {
#ifndef G9991
#ifdef LEGACY_RDP
        *rdd_emac = RDD_EMAC_ID_PCI;
#else
        *rdd_emac = RDD_EMAC_ID_WIFI;
#endif
        if (port >= rdpa_if_ssid0 && port <= rdpa_if_ssid15)
            *wifi_ssid = port - rdpa_if_ssid0;
        else /* For wlan0 or wlan1 we have no info about ssid */
            *wifi_ssid = 0;
        return 0;
#else
        *rdd_emac = 0;
        BDMF_TRACE_RET(BDMF_ERR_NOT_SUPPORTED, "Unexpected lan port %d\n", (int)port);
#endif
    }
    if (port >= rdpa_if_lag0 && port <= rdpa_if_lag4)
    {
        *rdd_emac = RDD_EMAC_ID_0 + (port - rdpa_if_lag0);
        *wifi_ssid = 0;
        return 0;
    }
    /* error case. assign some meaningful values anyway */
    *rdd_emac = RDD_EMAC_ID_START;
    *wifi_ssid = 0;
    BDMF_TRACE_RET(BDMF_ERR_NOT_SUPPORTED, "Unexpected lan port %d (%s)\n", (int)port,
        bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, port));
}

static inline rdd_emac_id_t rdpa_lan_phys_port_to_rdd_lan_mac(int phys_port)
{
#if defined(WL4908)
    return RDD_EMAC_ID_0 + (phys_port == 7 ? 4 : phys_port);
#else
    return RDD_EMAC_ID_0 + (phys_port == 7 ? 6 : phys_port);
#endif
}

static inline rdpa_if rdd_lan_mac_to_rdpa_if(rdd_emac_id_t rdd_emac)
{
#if defined(DSL_63138) || defined(DSL_63148)
    if (rdd_emac >= RDD_EMAC_ID_0 && rdd_emac <= RDD_EMAC_ID_7)
#else
    if (rdd_emac >= RDD_EMAC_ID_0 && rdd_emac < RDD_EMAC_ID_COUNT)
#endif
    {
        return rdpa_if_lan0 + (rdd_emac - RDD_EMAC_ID_0);
    }
    /* FIXME - Following needs to be taken care for ssid and lag and G9991 */
#if !defined(G9991) && !defined(DSL_63138) && !defined(DSL_63148)
#ifdef LEGACY_RDP
    if (rdd_emac == RDD_EMAC_ID_PCI)
#else
    if (rdd_emac == RDD_EMAC_ID_WIFI)
#endif
        return rdpa_if_wlan0;
#endif
    /* error case. assign some meaningful values anyway */
    BDMF_TRACE_ERR("Unexpected rdd_emac port %d\n", (int)rdd_emac);
    return rdpa_if_none;
}
#endif /* _RDPA_RDD_INLINE_H_ */

