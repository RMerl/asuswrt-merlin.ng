/*
    <:copyright-BRCM:2013:DUAL/GPL:standard
    
       Copyright (c) 2013 Broadcom 
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

#include "rdd.h"
#include "rdd_crc.h"

uint8_t g_static_mac_counter[RDD_MAC_TABLE_SIZE + RDD_MAC_TABLE_CAM_SIZE - 1];
uint8_t g_local_switching_filters_mode[LILAC_RDD_NUMBER_OF_BRIDGE_PORTS + 1];
BL_LILAC_RDD_ETHER_TYPE_FILTER_MATRIX_DTS *g_ether_type_filter_mode;

extern uint32_t g_mac_table_size;
extern uint32_t g_mac_table_search_depth;
extern RDD_64_BIT_TABLE_CFG g_hash_table_cfg[BL_LILAC_RDD_MAX_HASH_TABLE];

BL_LILAC_RDD_ERROR_DTE rdd_vlan_command_config ( rdpa_traffic_dir                        xi_direction,
                                                 rdd_vlan_command_params                 *xi_vlan_command_params )
{
    return (BL_LILAC_RDD_OK);
}

#if defined(LEGACY_RDP)
BL_LILAC_RDD_ERROR_DTE rdd_us_vlan_aggregation_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE       xi_bridge_port,
                                                        BL_LILAC_RDD_AGGREGATION_MODE_DTE  xi_vlan_aggregation_mode )
{
    return (BL_LILAC_RDD_OK);
}

BL_LILAC_RDD_ERROR_DTE rdd_vlan_switching_config ( BL_LILAC_RDD_VLAN_SWITCHING_CONFIG_DTE  xi_vlan_switching_mode,
                                                   BL_LILAC_RDD_VLAN_BINDING_CONFIG_DTE    xi_vlan_binding_mode )
{
    return (BL_LILAC_RDD_OK);
}

BL_LILAC_RDD_ERROR_DTE rdd_lan_vid_entry_add ( RDD_LAN_VID_PARAMS  *xi_lan_vid_params_ptr,
                                               uint32_t            *xo_entry_index )
{
    return ( BL_LILAC_RDD_ERROR_CAM_LOOKUP_TABLE_FULL );
}

BL_LILAC_RDD_ERROR_DTE rdd_lan_vid_entry_delete ( uint32_t  xi_entry_index )
{
    return (BL_LILAC_RDD_OK);
}
#endif

BL_LILAC_RDD_ERROR_DTE rdd_lan_vid_entry_modify ( uint32_t            xi_entry_index,
                                                  RDD_LAN_VID_PARAMS  *xi_lan_vid_params_ptr )
{
    return (BL_LILAC_RDD_OK);
}

BL_LILAC_RDD_ERROR_DTE rdd_wan_vid_config ( uint8_t   xi_wan_vid_index,
                                            uint16_t  xi_wan_vid )
{
    return (BL_LILAC_RDD_OK);
}

#if defined(LEGACY_RDP)
BL_LILAC_RDD_ERROR_DTE rdd_dscp_to_pbits_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_port,
                                                  uint32_t                      xi_dscp,
                                                  uint32_t                      xi_pbits )
{
    return (BL_LILAC_RDD_OK);
}

BL_LILAC_RDD_ERROR_DTE rdd_pbits_to_pbits_config ( uint32_t  xi_table_number,
                                                   uint32_t  xi_input_pbits,
                                                   uint32_t  xi_output_pbits )
{
    return (BL_LILAC_RDD_OK);
}
#else
void rdd_dscp_to_pbits_cfg(rdpa_traffic_dir direction, rdd_vport_id_t vport_id, uint32_t dscp, uint32_t pbits)
{
    return;
}

int rdd_pbits_to_pbits_config ( uint32_t  xi_table_number,
                                uint32_t  xi_input_pbits,
                                uint32_t  xi_output_pbits )
{
    return BDMF_ERR_OK;
}
#endif

BL_LILAC_RDD_ERROR_DTE rdd_tpid_detect_filter_value_config ( rdpa_traffic_dir  xi_direction,
                                                             uint16_t          tpid_detect_filter_value )
{
    return (BL_LILAC_RDD_OK);
}

static int32_t rdd_ingress_filter_cfg_entry_get(BL_LILAC_RDD_BRIDGE_PORT_DTE xi_bridge_port,
                                                BL_LILAC_RDD_SUBNET_ID_DTE xi_subnet_id,
                                                RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DTS **p_filters_cfg_entry_ptr)
{
    RDD_DS_INGRESS_FILTERS_CONFIGURATION_TABLE_DTS *ds_filters_cfg_table_ptr;
    RDD_US_INGRESS_FILTERS_CONFIGURATION_TABLE_DTS *us_filters_cfg_table_ptr;
    int32_t bridge_port_index;

    if ((bridge_port_index = rdd_bridge_port_to_port_index(xi_bridge_port, xi_subnet_id)) < 0)
        return bridge_port_index;

    if BL_LILAC_RDD_IS_WAN_BRIDGE_PORT(xi_bridge_port)
    {
        ds_filters_cfg_table_ptr = RDD_DS_INGRESS_FILTERS_CONFIGURATION_TABLE_PTR();
        *p_filters_cfg_entry_ptr = &(ds_filters_cfg_table_ptr->entry[bridge_port_index]);
    }
    else
    {
        us_filters_cfg_table_ptr = RDD_US_INGRESS_FILTERS_CONFIGURATION_TABLE_PTR();
        *p_filters_cfg_entry_ptr = &(us_filters_cfg_table_ptr->entry[bridge_port_index - BL_LILAC_RDD_LAN0_BRIDGE_PORT]);
    }

    return bridge_port_index;
}

static int32_t rdd_ingress_filter_cfg_and_cam_get(BL_LILAC_RDD_BRIDGE_PORT_DTE xi_bridge_port,
                                                  BL_LILAC_RDD_SUBNET_ID_DTE xi_subnet_id,
                                                  RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DTS **p_filters_cfg_entry_ptr,
                                                  RDD_INGRESS_FILTERS_LOOKUP_ENTRY_DTS **p_filters_cam_entry_ptr,
                                                  RDD_INGRESS_FILTERS_PARAMETER_ENTRY_DTS **p_filters_cam_parameter_entry_ptr,
                                                  uint32_t filter_number)
{
    RDD_DS_INGRESS_FILTERS_CONFIGURATION_TABLE_DTS  *ds_filters_cfg_table_ptr;
    RDD_US_INGRESS_FILTERS_CONFIGURATION_TABLE_DTS  *us_filters_cfg_table_ptr;
    RDD_DS_INGRESS_FILTERS_LOOKUP_TABLE_DTS         *ds_filters_cam_table_ptr;
    RDD_US_INGRESS_FILTERS_LOOKUP_TABLE_DTS         *us_filters_cam_table_ptr;
    RDD_DS_INGRESS_FILTERS_PARAMETER_TABLE_DTS      *ds_filters_cam_parameter_table_ptr;
    RDD_US_INGRESS_FILTERS_PARAMETER_TABLE_DTS      *us_filters_cam_parameter_table_ptr;
    int32_t                                         bridge_port_index;

    if ((bridge_port_index = rdd_bridge_port_to_port_index(xi_bridge_port, xi_subnet_id)) < 0)
        return bridge_port_index;

    if BL_LILAC_RDD_IS_WAN_BRIDGE_PORT(xi_bridge_port)
    {
        ds_filters_cfg_table_ptr = RDD_DS_INGRESS_FILTERS_CONFIGURATION_TABLE_PTR();
        ds_filters_cam_table_ptr = RDD_DS_INGRESS_FILTERS_LOOKUP_TABLE_PTR();
        ds_filters_cam_parameter_table_ptr = RDD_DS_INGRESS_FILTERS_PARAMETER_TABLE_PTR();

        *p_filters_cfg_entry_ptr = &(ds_filters_cfg_table_ptr->entry[bridge_port_index]);
        *p_filters_cam_entry_ptr = &(ds_filters_cam_table_ptr->entry[bridge_port_index][filter_number]);
        *p_filters_cam_parameter_entry_ptr = &(ds_filters_cam_parameter_table_ptr->entry[bridge_port_index][filter_number]);
    }
    else
    {
        us_filters_cfg_table_ptr = RDD_US_INGRESS_FILTERS_CONFIGURATION_TABLE_PTR();
        us_filters_cam_table_ptr = RDD_US_INGRESS_FILTERS_LOOKUP_TABLE_PTR();
        us_filters_cam_parameter_table_ptr = RDD_US_INGRESS_FILTERS_PARAMETER_TABLE_PTR();

        *p_filters_cfg_entry_ptr = &(us_filters_cfg_table_ptr->entry[bridge_port_index - BL_LILAC_RDD_LAN0_BRIDGE_PORT]);
        *p_filters_cam_entry_ptr = &(us_filters_cam_table_ptr->entry[bridge_port_index - BL_LILAC_RDD_LAN0_BRIDGE_PORT][filter_number]);
        *p_filters_cam_parameter_entry_ptr = &(us_filters_cam_parameter_table_ptr->entry[bridge_port_index - BL_LILAC_RDD_LAN0_BRIDGE_PORT][filter_number]);
    }

    return bridge_port_index;
}

static inline void f_rdd_ingress_filter_enable(RDD_INGRESS_FILTERS_LOOKUP_ENTRY_DTS *filters_cam_entry_ptr)
{
    RDD_INGRESS_FILTERS_LOOKUP_ENTRY_PTAG_WRITE(0, filters_cam_entry_ptr);
    RDD_INGRESS_FILTERS_LOOKUP_ENTRY_PTAG_MASK_WRITE(0, filters_cam_entry_ptr);
}

static inline void f_rdd_ingress_filter_disable(RDD_INGRESS_FILTERS_LOOKUP_ENTRY_DTS *filters_cam_entry_ptr)
{
    RDD_INGRESS_FILTERS_LOOKUP_ENTRY_PTAG_WRITE(1, filters_cam_entry_ptr);
    RDD_INGRESS_FILTERS_LOOKUP_ENTRY_PTAG_MASK_WRITE(0, filters_cam_entry_ptr);
}

BL_LILAC_RDD_ERROR_DTE rdd_tpid_detect_filter_config(BL_LILAC_RDD_BRIDGE_PORT_DTE xi_bridge_port,
                                                     BL_LILAC_RDD_SUBNET_ID_DTE xi_subnet_id,
                                                     BL_LILAC_RDD_FILTER_MODE_DTE xi_tpid_filter_mode)
{
    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DTS *filters_cfg_entry_ptr;
    int32_t bridge_port_index;

    if ((bridge_port_index = rdd_ingress_filter_cfg_entry_get(xi_bridge_port, xi_subnet_id, &filters_cfg_entry_ptr)) < 0)
        return (BL_LILAC_RDD_ERROR_ILLEGAL_BRIDGE_PORT_ID);

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_TPID_DETECT_FILTER_WRITE(xi_tpid_filter_mode, filters_cfg_entry_ptr);

    return (BL_LILAC_RDD_OK);
}

BL_LILAC_RDD_ERROR_DTE rdd_dhcp_filter_config(BL_LILAC_RDD_BRIDGE_PORT_DTE xi_bridge_port,
                                              BL_LILAC_RDD_SUBNET_ID_DTE xi_subnet_id,
                                              BL_LILAC_RDD_FILTER_MODE_DTE xi_dhcp_filter_mode)
{
    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DTS *filters_cfg_entry_ptr;
    int32_t bridge_port_index;
    uint32_t filters_mode;
    uint32_t filters_cfg_entry;

    if ((bridge_port_index = rdd_ingress_filter_cfg_entry_get(xi_bridge_port, xi_subnet_id, &filters_cfg_entry_ptr)) < 0)
        return (BL_LILAC_RDD_ERROR_ILLEGAL_BRIDGE_PORT_ID);

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_READ(filters_cfg_entry, filters_cfg_entry_ptr);

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DHCP_INGRESS_FILTER_L_WRITE(filters_cfg_entry, xi_dhcp_filter_mode);

    filters_mode = RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_MLD_INGRESS_FILTER_L_READ(filters_cfg_entry) |
                   RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_BYPASS_L_READ(filters_cfg_entry);

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_LOCAL_SWITCHING_INGRESS_FILTERS_L_WRITE(filters_cfg_entry, g_local_switching_filters_mode[bridge_port_index] && filters_mode);
    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_WRITE(filters_cfg_entry, filters_cfg_entry_ptr);

    return (BL_LILAC_RDD_OK);
}

BL_LILAC_RDD_ERROR_DTE rdd_mld_filter_config(BL_LILAC_RDD_BRIDGE_PORT_DTE xi_bridge_port,
                                             BL_LILAC_RDD_SUBNET_ID_DTE xi_subnet_id,
                                             BL_LILAC_RDD_FILTER_MODE_DTE xi_mld_filter_mode)
{
    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DTS *filters_cfg_entry_ptr;
    int32_t bridge_port_index;
    uint32_t filters_mode;
    uint32_t filters_cfg_entry;

    if ((bridge_port_index = rdd_ingress_filter_cfg_entry_get(xi_bridge_port, xi_subnet_id, &filters_cfg_entry_ptr)) < 0)
        return (BL_LILAC_RDD_ERROR_ILLEGAL_BRIDGE_PORT_ID);

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_READ(filters_cfg_entry, filters_cfg_entry_ptr);
    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_MLD_INGRESS_FILTER_L_WRITE(filters_cfg_entry, xi_mld_filter_mode);

    filters_mode = RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_BYPASS_L_READ(filters_cfg_entry) | xi_mld_filter_mode;

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_LOCAL_SWITCHING_INGRESS_FILTERS_L_WRITE(filters_cfg_entry, g_local_switching_filters_mode[bridge_port_index] && filters_mode);
    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_WRITE(filters_cfg_entry, filters_cfg_entry_ptr);

    return (BL_LILAC_RDD_OK);
}

BL_LILAC_RDD_ERROR_DTE rdd_igmp_filter_config(BL_LILAC_RDD_BRIDGE_PORT_DTE xi_bridge_port,
                                              BL_LILAC_RDD_SUBNET_ID_DTE xi_subnet_id,
                                              BL_LILAC_RDD_FILTER_MODE_DTE xi_igmp_filter_mode,
                                              BL_LILAC_RDD_FILTER_ACTION_DTE xi_filter_action)
{
    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DTS *filters_cfg_entry_ptr;
    RDD_INGRESS_FILTERS_LOOKUP_ENTRY_DTS *filters_cam_entry_ptr;
    RDD_INGRESS_FILTERS_PARAMETER_ENTRY_DTS *filters_cam_parameter_entry_ptr;
    int32_t bridge_port_index;
    uint32_t filters_mode;
    uint32_t filters_cfg_entry;

    bridge_port_index = rdd_ingress_filter_cfg_and_cam_get(xi_bridge_port, xi_subnet_id,
                                                           &filters_cfg_entry_ptr,
                                                           &filters_cam_entry_ptr,
                                                           &filters_cam_parameter_entry_ptr,
                                                           BL_LILAC_RDD_IGMP_FILTER_NUMBER);
    if (bridge_port_index < 0)
        return (BL_LILAC_RDD_ERROR_ILLEGAL_BRIDGE_PORT_ID);

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_READ(filters_cfg_entry, filters_cfg_entry_ptr);

    if (xi_igmp_filter_mode == BL_LILAC_RDD_FILTER_ENABLE)
    {
        f_rdd_ingress_filter_enable(filters_cam_entry_ptr);

        if (xi_filter_action == BL_LILAC_RDD_FILTER_ACTION_CPU_TRAP)
        {
            RDD_INGRESS_FILTERS_PARAMETER_ENTRY_PARAMETER_WRITE(rdpa_cpu_rx_reason_igmp,
                filters_cam_parameter_entry_ptr);
            RDD_INGRESS_FILTERS_PARAMETER_ENTRY_CPU_TRAP_WRITE(LILAC_RDD_TRUE, filters_cam_parameter_entry_ptr);
            RDD_INGRESS_FILTERS_PARAMETER_ENTRY_DROP_WRITE(LILAC_RDD_FALSE, filters_cam_parameter_entry_ptr);
        }
        else
        {
            RDD_INGRESS_FILTERS_PARAMETER_ENTRY_DROP_WRITE(LILAC_RDD_TRUE, filters_cam_parameter_entry_ptr);
            RDD_INGRESS_FILTERS_PARAMETER_ENTRY_PARAMETER_WRITE(0, filters_cam_parameter_entry_ptr);
            RDD_INGRESS_FILTERS_PARAMETER_ENTRY_CPU_TRAP_WRITE(LILAC_RDD_FALSE, filters_cam_parameter_entry_ptr);
        }
        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IGMP_FILTER_L_WRITE(filters_cfg_entry, BL_LILAC_RDD_FILTER_ENABLE);
        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_BYPASS_L_WRITE(filters_cfg_entry, BL_LILAC_RDD_FILTER_ENABLE);
    }
    else
    {
        f_rdd_ingress_filter_disable(filters_cam_entry_ptr);

        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IGMP_FILTER_L_WRITE(filters_cfg_entry, BL_LILAC_RDD_FILTER_DISABLE);

        filters_mode = RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_MLD_INGRESS_FILTER_L_READ(filters_cfg_entry) |
                       RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IGMP_FILTER_L_READ(filters_cfg_entry) |
                       RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_ICMPV6_FILTER_L_READ(filters_cfg_entry) |
                       RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_ETHERTYPE_FILTER_L_READ(filters_cfg_entry) |
                       RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_BROADCAST_FILTER_L_READ(filters_cfg_entry) |
                       RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_MULTICAST_FILTER_L_READ(filters_cfg_entry);

        if (filters_mode == 0)
            RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_BYPASS_L_WRITE(filters_cfg_entry, BL_LILAC_RDD_FILTER_DISABLE);
    }

    /* Update local switching filter mode */
    filters_mode = RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_MLD_INGRESS_FILTER_L_READ(filters_cfg_entry) |
                   RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_BYPASS_L_READ(filters_cfg_entry);

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_LOCAL_SWITCHING_INGRESS_FILTERS_L_WRITE(filters_cfg_entry, g_local_switching_filters_mode[bridge_port_index] && filters_mode);
    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_WRITE(filters_cfg_entry, filters_cfg_entry_ptr);

    return (BL_LILAC_RDD_OK);
}

BL_LILAC_RDD_ERROR_DTE rdd_icmpv6_filter_config(BL_LILAC_RDD_BRIDGE_PORT_DTE xi_bridge_port,
                                                BL_LILAC_RDD_SUBNET_ID_DTE xi_subnet_id,
                                                BL_LILAC_RDD_FILTER_MODE_DTE xi_icmpv6_filter_mode)
{
    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DTS *filters_cfg_entry_ptr;
    RDD_INGRESS_FILTERS_LOOKUP_ENTRY_DTS *filters_cam_entry_ptr;
    RDD_INGRESS_FILTERS_PARAMETER_ENTRY_DTS *filters_cam_parameter_entry_ptr;
    int32_t bridge_port_index;
    uint32_t filters_mode;
    uint32_t filters_cfg_entry;

    bridge_port_index = rdd_ingress_filter_cfg_and_cam_get(xi_bridge_port, xi_subnet_id,
                                                           &filters_cfg_entry_ptr,
                                                           &filters_cam_entry_ptr,
                                                           &filters_cam_parameter_entry_ptr,
                                                           BL_LILAC_RDD_ICMPV6_FILTER_NUMBER);
    if (bridge_port_index < 0)
        return (BL_LILAC_RDD_ERROR_ILLEGAL_BRIDGE_PORT_ID);

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_READ(filters_cfg_entry, filters_cfg_entry_ptr);

    if (xi_icmpv6_filter_mode == BL_LILAC_RDD_FILTER_ENABLE)
    {
        f_rdd_ingress_filter_enable(filters_cam_entry_ptr);

        RDD_INGRESS_FILTERS_PARAMETER_ENTRY_PARAMETER_WRITE(rdpa_cpu_rx_reason_icmpv6,
            filters_cam_parameter_entry_ptr);
        RDD_INGRESS_FILTERS_PARAMETER_ENTRY_CPU_TRAP_WRITE(LILAC_RDD_TRUE, filters_cam_parameter_entry_ptr);
        RDD_INGRESS_FILTERS_PARAMETER_ENTRY_DROP_WRITE(LILAC_RDD_FALSE, filters_cam_parameter_entry_ptr);
        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_ICMPV6_FILTER_L_WRITE(filters_cfg_entry, BL_LILAC_RDD_FILTER_ENABLE);
        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_BYPASS_L_WRITE(filters_cfg_entry, BL_LILAC_RDD_FILTER_ENABLE);
    }
    else
    {
        f_rdd_ingress_filter_disable(filters_cam_entry_ptr);
        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_ICMPV6_FILTER_L_WRITE(filters_cfg_entry, BL_LILAC_RDD_FILTER_DISABLE);

        filters_mode = RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_MLD_INGRESS_FILTER_L_READ(filters_cfg_entry) |
                       RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IGMP_FILTER_L_READ(filters_cfg_entry) |
                       RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_ICMPV6_FILTER_L_READ(filters_cfg_entry) |
                       RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_ETHERTYPE_FILTER_L_READ(filters_cfg_entry) |
                       RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_BROADCAST_FILTER_L_READ(filters_cfg_entry) |
                       RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_MULTICAST_FILTER_L_READ(filters_cfg_entry);

        if (filters_mode == 0)
            RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_BYPASS_L_WRITE(filters_cfg_entry, BL_LILAC_RDD_FILTER_DISABLE);
    }

    /* Update local switching filter mode */
    filters_mode = RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_MLD_INGRESS_FILTER_L_READ(filters_cfg_entry) |
                   RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_BYPASS_L_READ(filters_cfg_entry);

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_LOCAL_SWITCHING_INGRESS_FILTERS_L_WRITE(filters_cfg_entry, g_local_switching_filters_mode[bridge_port_index] && filters_mode);

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_WRITE(filters_cfg_entry, filters_cfg_entry_ptr);

    return (BL_LILAC_RDD_OK);
}

BL_LILAC_RDD_ERROR_DTE rdd_ether_type_filter_config(BL_LILAC_RDD_BRIDGE_PORT_DTE xi_bridge_port,
                                                    BL_LILAC_RDD_SUBNET_ID_DTE xi_subnet_id,
                                                    BL_LILAC_RDD_FILTER_MODE_DTE xi_ether_type_filter_mode,
                                                    BL_LILAC_RDD_ETHER_TYPE_FILTER_NUMBER_DTE xi_ether_type_filter_number,
                                                    BL_LILAC_RDD_FILTER_ACTION_DTE xi_ether_type_action)
{
    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DTS *filters_cfg_entry_ptr;
    RDD_INGRESS_FILTERS_LOOKUP_ENTRY_DTS *filters_cam_entry_ptr;
    RDD_INGRESS_FILTERS_PARAMETER_ENTRY_DTS *filters_cam_parameter_entry_ptr;
    int32_t bridge_port_index;
    uint32_t filters_mode;
    uint32_t filters_cfg_entry;
    int32_t ether_type_filter_index;
    uint8_t cpu_reason;

    bridge_port_index = rdd_ingress_filter_cfg_and_cam_get(xi_bridge_port, xi_subnet_id,
                                                           &filters_cfg_entry_ptr,
                                                           &filters_cam_entry_ptr,
                                                           &filters_cam_parameter_entry_ptr,
                                                           xi_ether_type_filter_number);
    if (bridge_port_index < 0)
        return (BL_LILAC_RDD_ERROR_ILLEGAL_BRIDGE_PORT_ID);

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_READ(filters_cfg_entry, filters_cfg_entry_ptr);

    if (xi_ether_type_filter_mode == BL_LILAC_RDD_FILTER_ENABLE)
    {
        if (xi_ether_type_action == BL_LILAC_RDD_FILTER_ACTION_CPU_TRAP)
        {
            cpu_reason = rdpa_cpu_rx_reason_etype_udef_0 + xi_ether_type_filter_number -
                BL_LILAC_RDD_ETHER_TYPE_FILTER_USER_0;
            RDD_INGRESS_FILTERS_PARAMETER_ENTRY_PARAMETER_WRITE(cpu_reason, filters_cam_parameter_entry_ptr);
            RDD_INGRESS_FILTERS_PARAMETER_ENTRY_CPU_TRAP_WRITE(LILAC_RDD_TRUE, filters_cam_parameter_entry_ptr);
            RDD_INGRESS_FILTERS_PARAMETER_ENTRY_DROP_WRITE(LILAC_RDD_FALSE, filters_cam_parameter_entry_ptr);
        }
        else if (xi_ether_type_action == BL_LILAC_RDD_FILTER_ACTION_DROP)
        {
            RDD_INGRESS_FILTERS_PARAMETER_ENTRY_DROP_WRITE(LILAC_RDD_TRUE, filters_cam_parameter_entry_ptr);
            RDD_INGRESS_FILTERS_PARAMETER_ENTRY_PARAMETER_WRITE(0, filters_cam_parameter_entry_ptr);
            RDD_INGRESS_FILTERS_PARAMETER_ENTRY_CPU_TRAP_WRITE(LILAC_RDD_FALSE, filters_cam_parameter_entry_ptr);
        }

        f_rdd_ingress_filter_enable(filters_cam_entry_ptr);

        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_ETHERTYPE_FILTER_L_WRITE(filters_cfg_entry, BL_LILAC_RDD_FILTER_ENABLE);
        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_BYPASS_L_WRITE(filters_cfg_entry, BL_LILAC_RDD_FILTER_ENABLE);

        g_ether_type_filter_mode->entry[xi_bridge_port][xi_subnet_id][xi_ether_type_filter_number - BL_LILAC_RDD_ETHER_TYPE_FILTER_USER_0] = BL_LILAC_RDD_FILTER_ENABLE;
    }
    else
    {
        f_rdd_ingress_filter_disable(filters_cam_entry_ptr);

        g_ether_type_filter_mode->entry[xi_bridge_port][xi_subnet_id][xi_ether_type_filter_number - BL_LILAC_RDD_ETHER_TYPE_FILTER_USER_0] = BL_LILAC_RDD_FILTER_DISABLE;

        for (ether_type_filter_index = BL_LILAC_RDD_ETHER_TYPE_FILTER_USER_0; ether_type_filter_index <= BL_LILAC_RDD_ETHER_TYPE_FILTER_802_1AG_CFM; ether_type_filter_index++)
        {
            if (g_ether_type_filter_mode->entry[xi_bridge_port][xi_subnet_id][ether_type_filter_index - BL_LILAC_RDD_ETHER_TYPE_FILTER_USER_0] == BL_LILAC_RDD_FILTER_ENABLE)
                break;
        }

        if (ether_type_filter_index > BL_LILAC_RDD_ETHER_TYPE_FILTER_802_1AG_CFM)
        {
            RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_ETHERTYPE_FILTER_L_WRITE(filters_cfg_entry, BL_LILAC_RDD_FILTER_DISABLE);

            filters_mode = RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_MLD_INGRESS_FILTER_L_READ(filters_cfg_entry) |
                           RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IGMP_FILTER_L_READ(filters_cfg_entry) |
                           RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_ICMPV6_FILTER_L_READ(filters_cfg_entry) |
                           RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_ETHERTYPE_FILTER_L_READ(filters_cfg_entry) |
                           RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_BROADCAST_FILTER_L_READ(filters_cfg_entry) |
                           RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_MULTICAST_FILTER_L_READ(filters_cfg_entry);

            if (filters_mode == 0)
                RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_BYPASS_L_WRITE(filters_cfg_entry, BL_LILAC_RDD_FILTER_DISABLE);
        }
    }

    /* Update local switching filter mode */
    filters_mode = RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_MLD_INGRESS_FILTER_L_READ(filters_cfg_entry) |
                   RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_BYPASS_L_READ(filters_cfg_entry);

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_LOCAL_SWITCHING_INGRESS_FILTERS_L_WRITE(filters_cfg_entry, g_local_switching_filters_mode[bridge_port_index] && filters_mode);

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_WRITE(filters_cfg_entry, filters_cfg_entry_ptr);

    return (BL_LILAC_RDD_OK);
}

BL_LILAC_RDD_ERROR_DTE rdd_broadcast_filter_config(BL_LILAC_RDD_BRIDGE_PORT_DTE xi_bridge_port,
                                                   BL_LILAC_RDD_SUBNET_ID_DTE xi_subnet_id,
                                                   BL_LILAC_RDD_FILTER_MODE_DTE xi_broadcast_filter_mode,
                                                   BL_LILAC_RDD_FILTER_ACTION_DTE xi_filter_action)
{
    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DTS *filters_cfg_entry_ptr;
    RDD_INGRESS_FILTERS_LOOKUP_ENTRY_DTS *filters_cam_entry_ptr;
    RDD_INGRESS_FILTERS_PARAMETER_ENTRY_DTS *filters_cam_parameter_entry_ptr;
    int32_t bridge_port_index;
    uint32_t filters_mode;
    uint32_t filters_cfg_entry;

    bridge_port_index = rdd_ingress_filter_cfg_and_cam_get(xi_bridge_port, xi_subnet_id,
                                                           &filters_cfg_entry_ptr,
                                                           &filters_cam_entry_ptr,
                                                           &filters_cam_parameter_entry_ptr,
                                                           BL_LILAC_RDD_BROADCAST_FILTER_NUMBER);
    if (bridge_port_index < 0)
        return (BL_LILAC_RDD_ERROR_ILLEGAL_BRIDGE_PORT_ID);

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_READ(filters_cfg_entry, filters_cfg_entry_ptr);

    if (xi_broadcast_filter_mode == BL_LILAC_RDD_FILTER_ENABLE)
    {
        f_rdd_ingress_filter_enable(filters_cam_entry_ptr);

        if (xi_filter_action == BL_LILAC_RDD_FILTER_ACTION_CPU_TRAP)
        {
            RDD_INGRESS_FILTERS_PARAMETER_ENTRY_PARAMETER_WRITE(rdpa_cpu_rx_reason_bcast,
                filters_cam_parameter_entry_ptr);
            RDD_INGRESS_FILTERS_PARAMETER_ENTRY_CPU_TRAP_WRITE(LILAC_RDD_TRUE, filters_cam_parameter_entry_ptr);
            RDD_INGRESS_FILTERS_PARAMETER_ENTRY_DROP_WRITE(LILAC_RDD_FALSE, filters_cam_parameter_entry_ptr);
        }
        else
        {
            RDD_INGRESS_FILTERS_PARAMETER_ENTRY_DROP_WRITE(LILAC_RDD_TRUE, filters_cam_parameter_entry_ptr);
            RDD_INGRESS_FILTERS_PARAMETER_ENTRY_PARAMETER_WRITE(0, filters_cam_parameter_entry_ptr);
            RDD_INGRESS_FILTERS_PARAMETER_ENTRY_CPU_TRAP_WRITE(LILAC_RDD_FALSE, filters_cam_parameter_entry_ptr);
        }

        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_BROADCAST_FILTER_L_WRITE(filters_cfg_entry, BL_LILAC_RDD_FILTER_ENABLE);
        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_BYPASS_L_WRITE(filters_cfg_entry, BL_LILAC_RDD_FILTER_ENABLE);
    }
    else
    {
        f_rdd_ingress_filter_disable(filters_cam_entry_ptr);

        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_BROADCAST_FILTER_L_WRITE(filters_cfg_entry, BL_LILAC_RDD_FILTER_DISABLE);

        filters_mode = RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_MLD_INGRESS_FILTER_L_READ(filters_cfg_entry) |
                       RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IGMP_FILTER_L_READ(filters_cfg_entry) |
                       RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_ICMPV6_FILTER_L_READ(filters_cfg_entry) |
                       RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_ETHERTYPE_FILTER_L_READ(filters_cfg_entry) |
                       RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_BROADCAST_FILTER_L_READ(filters_cfg_entry) |
                       RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_MULTICAST_FILTER_L_READ(filters_cfg_entry);

        if (filters_mode == 0)
            RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_BYPASS_L_WRITE(filters_cfg_entry, BL_LILAC_RDD_FILTER_DISABLE);
    }

    /* Update local switching filter mode */
    filters_mode = RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_MLD_INGRESS_FILTER_L_READ(filters_cfg_entry) |
                   RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_BYPASS_L_READ(filters_cfg_entry);

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_LOCAL_SWITCHING_INGRESS_FILTERS_L_WRITE(filters_cfg_entry, g_local_switching_filters_mode[bridge_port_index] && filters_mode);
    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_WRITE(filters_cfg_entry, filters_cfg_entry_ptr);

    return (BL_LILAC_RDD_OK);
}

BL_LILAC_RDD_ERROR_DTE rdd_multicast_filter_config(BL_LILAC_RDD_BRIDGE_PORT_DTE xi_bridge_port,
                                                   BL_LILAC_RDD_SUBNET_ID_DTE xi_subnet_id,
                                                   BL_LILAC_RDD_FILTER_MODE_DTE xi_multicast_filter_mode,
                                                   BL_LILAC_RDD_FILTER_ACTION_DTE xi_filter_action)
{
    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DTS *filters_cfg_entry_ptr;
    RDD_INGRESS_FILTERS_LOOKUP_ENTRY_DTS *filters_cam_entry_ptr;
    RDD_INGRESS_FILTERS_PARAMETER_ENTRY_DTS *filters_cam_parameter_entry_ptr;
    int32_t bridge_port_index;
    uint32_t filters_mode;
    uint32_t filters_cfg_entry;

    bridge_port_index = rdd_ingress_filter_cfg_and_cam_get(xi_bridge_port, xi_subnet_id,
                                                           &filters_cfg_entry_ptr,
                                                           &filters_cam_entry_ptr,
                                                           &filters_cam_parameter_entry_ptr,
                                                           BL_LILAC_RDD_MULTICAST_FILTER_NUMBER);
    if (bridge_port_index < 0)
        return (BL_LILAC_RDD_ERROR_ILLEGAL_BRIDGE_PORT_ID);

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_READ(filters_cfg_entry, filters_cfg_entry_ptr);

    if (xi_multicast_filter_mode == BL_LILAC_RDD_FILTER_ENABLE)
    {
        f_rdd_ingress_filter_enable(filters_cam_entry_ptr);

        if (xi_filter_action == BL_LILAC_RDD_FILTER_ACTION_CPU_TRAP)
        {
            RDD_INGRESS_FILTERS_PARAMETER_ENTRY_PARAMETER_WRITE(rdpa_cpu_rx_reason_mcast, filters_cam_parameter_entry_ptr);
            RDD_INGRESS_FILTERS_PARAMETER_ENTRY_CPU_TRAP_WRITE(LILAC_RDD_TRUE, filters_cam_parameter_entry_ptr);

            RDD_INGRESS_FILTERS_PARAMETER_ENTRY_DROP_WRITE(LILAC_RDD_FALSE, filters_cam_parameter_entry_ptr);
        }
        else
        {
            RDD_INGRESS_FILTERS_PARAMETER_ENTRY_DROP_WRITE(LILAC_RDD_TRUE, filters_cam_parameter_entry_ptr);

            RDD_INGRESS_FILTERS_PARAMETER_ENTRY_PARAMETER_WRITE(0, filters_cam_parameter_entry_ptr);
            RDD_INGRESS_FILTERS_PARAMETER_ENTRY_CPU_TRAP_WRITE(LILAC_RDD_FALSE, filters_cam_parameter_entry_ptr);
        }

        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_MULTICAST_FILTER_L_WRITE(filters_cfg_entry, BL_LILAC_RDD_FILTER_ENABLE);
        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_BYPASS_L_WRITE(filters_cfg_entry, BL_LILAC_RDD_FILTER_ENABLE);
    }
    else
    {
        f_rdd_ingress_filter_disable(filters_cam_entry_ptr);

        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_MULTICAST_FILTER_L_WRITE(filters_cfg_entry, BL_LILAC_RDD_FILTER_DISABLE);

        filters_mode = RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_MLD_INGRESS_FILTER_L_READ(filters_cfg_entry) |
                       RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IGMP_FILTER_L_READ(filters_cfg_entry) |
                       RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_ICMPV6_FILTER_L_READ(filters_cfg_entry) |
                       RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_ETHERTYPE_FILTER_L_READ(filters_cfg_entry) |
                       RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_BROADCAST_FILTER_L_READ(filters_cfg_entry) |
                       RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_MULTICAST_FILTER_L_READ(filters_cfg_entry);

        if (filters_mode == 0)
            RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_BYPASS_L_WRITE(filters_cfg_entry, BL_LILAC_RDD_FILTER_DISABLE);
    }

    /* Update local switching filter mode */
    filters_mode = RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_MLD_INGRESS_FILTER_L_READ(filters_cfg_entry) |
                   RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_BYPASS_L_READ(filters_cfg_entry);

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_LOCAL_SWITCHING_INGRESS_FILTERS_L_WRITE(filters_cfg_entry, g_local_switching_filters_mode[bridge_port_index] && filters_mode);

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_WRITE(filters_cfg_entry, filters_cfg_entry_ptr);

    return (BL_LILAC_RDD_OK);
}

BL_LILAC_RDD_ERROR_DTE rdd_local_switching_filters_config(BL_LILAC_RDD_BRIDGE_PORT_DTE xi_bridge_port,
                                                          BL_LILAC_RDD_FILTER_MODE_DTE xi_local_switching_filters_mode)
{
    RDD_US_INGRESS_FILTERS_CONFIGURATION_TABLE_DTS *us_filters_cfg_table_ptr;
    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DTS *filters_cfg_entry_ptr;
    int32_t bridge_port_index;

    if BL_LILAC_RDD_IS_WAN_BRIDGE_PORT(xi_bridge_port)
        return (BL_LILAC_RDD_ERROR_ILLEGAL_BRIDGE_PORT_ID);

    if ((bridge_port_index = rdd_bridge_port_to_port_index(xi_bridge_port, 0)) < 0)
        return (BL_LILAC_RDD_ERROR_ILLEGAL_BRIDGE_PORT_ID);

    g_local_switching_filters_mode[bridge_port_index] = xi_local_switching_filters_mode;

    us_filters_cfg_table_ptr = RDD_US_INGRESS_FILTERS_CONFIGURATION_TABLE_PTR();

    filters_cfg_entry_ptr = &(us_filters_cfg_table_ptr->entry[bridge_port_index]);

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_LOCAL_SWITCHING_INGRESS_FILTERS_WRITE(xi_local_switching_filters_mode, filters_cfg_entry_ptr);

    return (BL_LILAC_RDD_OK);
}

BL_LILAC_RDD_ERROR_DTE rdd_ip_fragments_ingress_filter_config(BL_LILAC_RDD_BRIDGE_PORT_DTE xi_bridge_port,
                                                              BL_LILAC_RDD_SUBNET_ID_DTE xi_subnet_id,
                                                              BL_LILAC_RDD_FILTER_MODE_DTE xi_ip_fragments_filter_mode,
                                                              BL_LILAC_RDD_FILTER_ACTION_DTE xi_filter_action)
{
    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DTS *filters_cfg_entry_ptr;
    int32_t bridge_port_index;
    uint32_t header_error_filter;
    uint32_t filters_cfg_entry;

    if ((bridge_port_index = rdd_ingress_filter_cfg_entry_get(xi_bridge_port, xi_subnet_id, &filters_cfg_entry_ptr)) < 0)
        return (BL_LILAC_RDD_ERROR_ILLEGAL_BRIDGE_PORT_ID);

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_READ(filters_cfg_entry, filters_cfg_entry_ptr);

    if (xi_ip_fragments_filter_mode == BL_LILAC_RDD_FILTER_ENABLE)
    {
        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IP_FRAGMENT_INGRESS_FILTER_L_WRITE(filters_cfg_entry, BL_LILAC_RDD_FILTER_ENABLE);
        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IP_VALIDATION_FILTER_L_WRITE(filters_cfg_entry, BL_LILAC_RDD_FILTER_ENABLE);

        if (xi_filter_action == BL_LILAC_RDD_FILTER_ACTION_CPU_TRAP)
            RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IP_FRAGMENT_INGRESS_FILTER_TRAP_L_WRITE(filters_cfg_entry, BL_LILAC_RDD_FILTER_ENABLE);
        else
            RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IP_FRAGMENT_INGRESS_FILTER_TRAP_L_WRITE(filters_cfg_entry, BL_LILAC_RDD_FILTER_DISABLE);
    }
    else
    {
        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IP_FRAGMENT_INGRESS_FILTER_L_WRITE(filters_cfg_entry, BL_LILAC_RDD_FILTER_DISABLE);

        header_error_filter = RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_HEADER_ERROR_INGRESS_FILTER_L_READ(filters_cfg_entry);

        if (header_error_filter == 0)
            RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IP_VALIDATION_FILTER_L_WRITE(filters_cfg_entry, BL_LILAC_RDD_FILTER_DISABLE);
    }

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_WRITE(filters_cfg_entry, filters_cfg_entry_ptr);

    return (BL_LILAC_RDD_OK);
}

BL_LILAC_RDD_ERROR_DTE rdd_header_error_ingress_filter_config(BL_LILAC_RDD_BRIDGE_PORT_DTE xi_bridge_port,
                                                              BL_LILAC_RDD_SUBNET_ID_DTE xi_subnet_id,
                                                              BL_LILAC_RDD_FILTER_MODE_DTE xi_header_error_filter_mode,
                                                              BL_LILAC_RDD_FILTER_ACTION_DTE xi_filter_action)
{
    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DTS *filters_cfg_entry_ptr;
    int32_t bridge_port_index;
    uint32_t ip_fragment_error_filter;
    uint32_t filters_cfg_entry;

    if ((bridge_port_index = rdd_ingress_filter_cfg_entry_get(xi_bridge_port, xi_subnet_id, &filters_cfg_entry_ptr)) < 0)
        return (BL_LILAC_RDD_ERROR_ILLEGAL_BRIDGE_PORT_ID);

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_READ(filters_cfg_entry, filters_cfg_entry_ptr);

    if (xi_header_error_filter_mode == BL_LILAC_RDD_FILTER_ENABLE)
    {
        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_HEADER_ERROR_INGRESS_FILTER_L_WRITE(filters_cfg_entry, BL_LILAC_RDD_FILTER_ENABLE);
        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IP_VALIDATION_FILTER_L_WRITE(filters_cfg_entry, BL_LILAC_RDD_FILTER_ENABLE);

        if (xi_filter_action == BL_LILAC_RDD_FILTER_ACTION_CPU_TRAP)
            RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_HEADER_ERROR_INGRESS_FILTER_TRAP_L_WRITE(filters_cfg_entry, BL_LILAC_RDD_FILTER_ENABLE);
        else
            RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_HEADER_ERROR_INGRESS_FILTER_TRAP_L_WRITE(filters_cfg_entry, BL_LILAC_RDD_FILTER_DISABLE);
    }
    else
    {
        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_HEADER_ERROR_INGRESS_FILTER_L_WRITE(filters_cfg_entry, BL_LILAC_RDD_FILTER_DISABLE);

        ip_fragment_error_filter = RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IP_FRAGMENT_INGRESS_FILTER_L_READ(filters_cfg_entry);

        if (ip_fragment_error_filter == 0)
            RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IP_VALIDATION_FILTER_L_WRITE(filters_cfg_entry, BL_LILAC_RDD_FILTER_DISABLE);
    }

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_WRITE(filters_cfg_entry, filters_cfg_entry_ptr);

    return (BL_LILAC_RDD_OK);
}

BL_LILAC_RDD_ERROR_DTE rdd_bridge_flooding_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_ports_vector,
                                                    uint16_t                      xi_wifi_ssid_vector)
{
    return (BL_LILAC_RDD_OK);
}

void f_rdd_mac_entry_set_type_bit(uint32_t xi_entry_index,
                                  BL_LILAC_RDD_MAC_ENTRY_TYPE_DTE xi_entry_type)
{
    RDD_MAC_CONTEXT_TABLE_DTS *mac_context_table_ptr;
    RDD_MAC_CONTEXT_ENTRY_DTS *mac_context_entry_ptr;

    if (xi_entry_index < g_mac_table_size)
    {
        mac_context_table_ptr = (RDD_MAC_CONTEXT_TABLE_DTS *)(DEVICE_ADDRESS(RUNNER_COMMON_0_OFFSET) + MAC_CONTEXT_TABLE_ADDRESS);
    }
    else
    {
        mac_context_table_ptr = (RDD_MAC_CONTEXT_TABLE_DTS *)(DEVICE_ADDRESS(RUNNER_COMMON_0_OFFSET) + MAC_CONTEXT_TABLE_CAM_ADDRESS);
    }

    mac_context_entry_ptr = &(mac_context_table_ptr->entry[xi_entry_index % g_mac_table_size]);

    RDD_MAC_CONTEXT_ENTRY_MAC_TYPE_WRITE(xi_entry_type, mac_context_entry_ptr);
}

BL_LILAC_RDD_ERROR_DTE p_rdd_write_mac_extension_entry(uint32_t xi_entry_index,
                                                       uint8_t xi_extension_entry)
{
    RDD_MAC_EXTENSION_TABLE_DTS *mac_extension_table_ptr;
    RDD_MAC_EXTENSION_ENTRY_DTS *mac_extension_entry_ptr;

    if (xi_entry_index < g_mac_table_size)
    {
        mac_extension_table_ptr = (RDD_MAC_EXTENSION_TABLE_DTS *)(DEVICE_ADDRESS(RUNNER_COMMON_0_OFFSET) + MAC_EXTENSION_TABLE_ADDRESS);

        mac_extension_entry_ptr = &(mac_extension_table_ptr->entry[xi_entry_index]);
    }
    else
    {
        mac_extension_table_ptr = (RDD_MAC_EXTENSION_TABLE_DTS *)(DEVICE_ADDRESS(RUNNER_COMMON_0_OFFSET) + MAC_EXTENSION_TABLE_CAM_ADDRESS);

        mac_extension_entry_ptr = &(mac_extension_table_ptr->entry[xi_entry_index % g_mac_table_size]);
    }

    RDD_MAC_EXTENSION_ENTRY_EXTENSION_ENTRY_WRITE(xi_extension_entry, mac_extension_entry_ptr);

    return (BL_LILAC_RDD_OK);
}

static BL_LILAC_RDD_ERROR_DTE f_rdd_mac_entry_modify(RDD_MAC_PARAMS *xi_mac_params_ptr)
{
    BL_LILAC_RDD_ERROR_DTE rdd_error;
    uint8_t hash_entry[8];
    uint8_t context_entry[2];
    uint32_t entry_index;

    hash_entry[0] = (xi_mac_params_ptr->bridge_port >> 4) & 0x01;
    hash_entry[1] = (xi_mac_params_ptr->bridge_port << 4) & 0xF0;
    hash_entry[2] = xi_mac_params_ptr->mac_addr.b[0];
    hash_entry[3] = xi_mac_params_ptr->mac_addr.b[1];
    hash_entry[4] = xi_mac_params_ptr->mac_addr.b[2];
    hash_entry[5] = xi_mac_params_ptr->mac_addr.b[3];
    hash_entry[6] = xi_mac_params_ptr->mac_addr.b[4];
    hash_entry[7] = xi_mac_params_ptr->mac_addr.b[5];

    context_entry[0] = xi_mac_params_ptr->bridge_port;
    context_entry[1] = (xi_mac_params_ptr->sa_action << 0) | (xi_mac_params_ptr->da_action << 3) | LILAC_RDD_MAC_CONTEXT_ENTRY_TYPE_MASK;

    rdd_error = rdd_modify_hash_entry_64_bit(&g_hash_table_cfg[BL_LILAC_RDD_MAC_TABLE],
                                             hash_entry,
                                             context_entry,
                                             MAC_ENTRY_KEY_MASK_HIGH,
                                             MAC_ENTRY_KEY_MASK_LOW,
                                             MAC_ENTRY_INTERNAL_CONTEXT_MASK_HIGH,
                                             MAC_ENTRY_INTERNAL_CONTEXT_MASK_LOW,
                                             0,
                                             &entry_index);

    if (rdd_error)
        return (rdd_error);

    /* write extension entry (ssid or aggregation vid index) */
    if ((xi_mac_params_ptr->bridge_port != BL_LILAC_RDD_PCI_BRIDGE_PORT) && !(xi_mac_params_ptr->bridge_port >= BL_LILAC_RDD_MULTICAST_LAN0_BRIDGE_PORT))
    {
        if (xi_mac_params_ptr->aggregation_mode == BL_LILAC_RDD_AGGREGATION_MODE_DISABLED)
            xi_mac_params_ptr->extension_entry &= ~(1 << 7);
        else
            xi_mac_params_ptr->extension_entry |= (1 << 7);
    }

    rdd_error = p_rdd_write_mac_extension_entry(entry_index, xi_mac_params_ptr->extension_entry);

    return (rdd_error);
}

BL_LILAC_RDD_ERROR_DTE p_rdd_add_mac_entry_type_handle(bdmf_mac_t *xi_mac_addr,
                                                       BL_LILAC_RDD_BRIDGE_PORT_DTE xi_bridge_port,
                                                       BL_LILAC_RDD_AGGREGATION_MODE_DTE xi_aggregation_mode,
                                                       uint8_t xi_extension_entry,
                                                       BL_LILAC_RDD_MAC_FWD_ACTION_DTE xi_sa_action,
                                                       BL_LILAC_RDD_MAC_FWD_ACTION_DTE xi_da_action,
                                                       uint32_t xi_entry_index)
{
    RDD_MAC_PARAMS mac_params;
    BL_LILAC_RDD_ERROR_DTE rdd_error;
    uint32_t dummy_entry;
    
    rdd_mac_entry_get(xi_entry_index, &mac_params, &dummy_entry, &dummy_entry, &dummy_entry);

    if (xi_bridge_port == BL_LILAC_RDD_CPU_BRIDGE_PORT)
    {
        /* adding static entry - increment static counter */
        g_static_mac_counter[xi_entry_index]++;
        return (BL_LILAC_RDD_OK);
    }
    else if (mac_params.bridge_port == BL_LILAC_RDD_CPU_BRIDGE_PORT)
    {
        /* adding bridge entry to entry that was static - change entry parameters */
        memcpy(mac_params.mac_addr.b, xi_mac_addr->b, 6);
        mac_params.bridge_port = xi_bridge_port;
        mac_params.aggregation_mode = xi_aggregation_mode;
        mac_params.extension_entry = xi_extension_entry;
        mac_params.sa_action = xi_sa_action;
        mac_params.da_action = xi_da_action;

        rdd_error = f_rdd_mac_entry_modify(&mac_params);

        f_rdd_mac_entry_set_type_bit(xi_entry_index, BL_LILAC_RDD_BRIDGE_MAC_ADDRESS);

        return (rdd_error);
    }
    else
    {
        return (BL_LILAC_RDD_ERROR_MAC_ENTRY_EXISTS);
    }
}


BL_LILAC_RDD_ERROR_DTE p_rdd_delete_mac_entry_type_handle(bdmf_mac_t *xi_mac_addr,
                                                          uint32_t xi_entry_index,
                                                          BL_LILAC_RDD_MAC_ENTRY_TYPE_DTE xi_entry_type,
                                                          uint32_t *xo_type_handle_status)
{
    RDD_MAC_PARAMS mac_params;
    BL_LILAC_RDD_ERROR_DTE rdd_error;
    uint32_t dummy_entry;

    *xo_type_handle_status = LILAC_RDD_TRUE;

    rdd_mac_entry_get(xi_entry_index, &mac_params, &dummy_entry, &dummy_entry, &dummy_entry);

    if (xi_entry_type == BL_LILAC_RDD_BRIDGE_MAC_ADDRESS)
    {
        /* deleting bridge entry */
        if (mac_params.bridge_port == BL_LILAC_RDD_CPU_BRIDGE_PORT)
        {
            /* given mac not associated to bridge */
            return (BL_LILAC_RDD_ERROR_MAC_ENTRY_DOESNT_EXIST);
        }

        if (g_static_mac_counter[xi_entry_index] > 0)
        {
            /* entry is also static - remove bridge entry */
            memcpy(mac_params.mac_addr.b, xi_mac_addr->b, 6);
            mac_params.bridge_port = BL_LILAC_RDD_CPU_BRIDGE_PORT;
            mac_params.entry_type = 0;
            mac_params.aggregation_mode = 0;
            mac_params.extension_entry = 0;
            mac_params.sa_action = 0;
            mac_params.da_action = 0;
            rdd_error = f_rdd_mac_entry_modify(&mac_params);

            f_rdd_mac_entry_set_type_bit(xi_entry_index, BL_LILAC_RDD_STATIC_MAC_ADDRESS);

            return (rdd_error);
        }
    }
    else
    {
        /* deleting static entry */
        if (g_static_mac_counter[xi_entry_index] == 0)
        {
            /* entry isn't static */
            return (BL_LILAC_RDD_ERROR_MAC_ENTRY_DOESNT_EXIST);
        }

        if (g_static_mac_counter[xi_entry_index] > 1)
        {
            /* more than one static entry associated - decrement static counter */
            g_static_mac_counter[xi_entry_index]--;

            return (BL_LILAC_RDD_OK);
        }

        if (g_static_mac_counter[xi_entry_index] == 1 && mac_params.bridge_port != BL_LILAC_RDD_CPU_BRIDGE_PORT)
        {
            /* last associated static entry deleted - leave only bridge entry */
            g_static_mac_counter[xi_entry_index] = 0;
            return (BL_LILAC_RDD_OK);
        }
    }

    /* no condition reached, proceed with deleting entry from table */
    *xo_type_handle_status = LILAC_RDD_FALSE;
    return (BL_LILAC_RDD_OK);
}



BL_LILAC_RDD_ERROR_DTE f_rdd_mac_table_initialize(uint32_t xi_mac_table_size,
                                                  uint32_t xi_iptv_table_size)
{
    RUNNER_REGS_CFG_LKUP1_CFG hash_lkup_1_cfg_register;
    RUNNER_REGS_CFG_LKUP1_CAM_CFG hash_lkup_1_cam_cfg_register;
    RUNNER_REGS_CFG_LKUP_GLBL_MASK1_H hash_lkup_1_global_mask_high_register;
    RUNNER_REGS_CFG_LKUP_GLBL_MASK1_L hash_lkup_1_global_mask_low_register;
    RDD_64_BIT_TABLE_CFG *hash_table_cfg_ptr;
    uint32_t index;

    /* for 63138/63148 and 4908/62118, we have 64-entry MAC address hash table, and 32-entry CAM table
     * if one is to change table size and configuration, please also modify [DSL_63138/WL4908]/drivers/rdd/rdd_ih_defs.h */
    hash_lkup_1_cfg_register.base_address = (MAC_TABLE_ADDRESS >> 3);
    hash_lkup_1_cfg_register.table_size = xi_mac_table_size;
    /* the below value has to match with the defined in [DSL_63138/WL4908]/drivers/rdd/rdd_ih_defs.h */
    hash_lkup_1_cfg_register.max_hop = BL_LILAC_RDD_MAC_TABLE_MAX_HOP_4;
    hash_lkup_1_cfg_register.hash_type = LILAC_RDD_MAC_HASH_TYPE_CRC16;

    RUNNER_REGS_0_CFG_LKUP1_CFG_WRITE(hash_lkup_1_cfg_register);
    RUNNER_REGS_1_CFG_LKUP1_CFG_WRITE(hash_lkup_1_cfg_register);

    hash_lkup_1_cam_cfg_register.cam_en = LILAC_RDD_TRUE;
    hash_lkup_1_cam_cfg_register.base_address = (MAC_TABLE_CAM_ADDRESS >> 3);

    RUNNER_REGS_0_CFG_LKUP1_CAM_CFG_WRITE(hash_lkup_1_cam_cfg_register);
    RUNNER_REGS_1_CFG_LKUP1_CAM_CFG_WRITE(hash_lkup_1_cam_cfg_register);

    hash_lkup_1_global_mask_high_register.base_address = 0xFFFF;

    RUNNER_REGS_0_CFG_LKUP_GLBL_MASK1_H_WRITE(hash_lkup_1_global_mask_high_register);

    hash_lkup_1_global_mask_low_register.base_address = 0xFFFFFFFF;

    RUNNER_REGS_0_CFG_LKUP_GLBL_MASK1_L_WRITE(hash_lkup_1_global_mask_low_register);

    /* initialize global variables - MAC Table Size and Search Depth, the values should be translated from HW defined constants to real values */
    hash_table_cfg_ptr = &g_hash_table_cfg[BL_LILAC_RDD_MAC_TABLE];

    hash_table_cfg_ptr->hash_table_size = (1 << xi_mac_table_size) * 32;
    hash_table_cfg_ptr->hash_table_search_depth = (1 << BL_LILAC_RDD_MAC_TABLE_MAX_HOP_4);
    hash_table_cfg_ptr->hash_table_ptr = (RDD_64_BIT_TABLE_ENTRY_DTS *)(DEVICE_ADDRESS(RUNNER_COMMON_0_OFFSET) + MAC_TABLE_ADDRESS);

    hash_table_cfg_ptr->is_external_context = BL_LILAC_RDD_EXTERNAL_CONTEXT_ENABLE;
    hash_table_cfg_ptr->context_size = BL_LILAC_RDD_CONTEXT_16_BIT;
    hash_table_cfg_ptr->context_table_ptr = (uint8_t *)(DEVICE_ADDRESS(RUNNER_COMMON_0_OFFSET) + MAC_CONTEXT_TABLE_ADDRESS);
    hash_table_cfg_ptr->is_extension_cam = LILAC_RDD_TRUE;
    hash_table_cfg_ptr->cam_table_size = RDD_MAC_TABLE_CAM_SIZE - 1;
    hash_table_cfg_ptr->cam_table_ptr = (RDD_64_BIT_TABLE_ENTRY_DTS *)(DEVICE_ADDRESS(RUNNER_COMMON_0_OFFSET) + MAC_TABLE_CAM_ADDRESS);
    hash_table_cfg_ptr->cam_context_table_ptr = (uint8_t *)(DEVICE_ADDRESS(RUNNER_COMMON_0_OFFSET) + MAC_CONTEXT_TABLE_CAM_ADDRESS);

    g_mac_table_size = (1 << xi_mac_table_size) * 32;
    g_mac_table_search_depth = (1 << BL_LILAC_RDD_MAC_TABLE_MAX_HOP_4);

    /* reset static mac counters */
    for (index = 0; index < (RDD_MAC_TABLE_SIZE + RDD_MAC_TABLE_CAM_SIZE - 1); index++ )
    {
        g_static_mac_counter[index] = 0;
    }

    return (BL_LILAC_RDD_OK);
}

BL_LILAC_RDD_ERROR_DTE rdd_mac_entry_add(RDD_MAC_PARAMS *xi_mac_params_ptr,
                                         uint32_t *xo_mac_entry_index)
{
    BL_LILAC_RDD_ERROR_DTE rdd_error;
    uint8_t hash_entry[8];
    uint8_t context_entry[2];
    unsigned long flags;

    /* build hash entry */
    hash_entry[0] = 0;
    hash_entry[1] = 0;
    hash_entry[2] = xi_mac_params_ptr->mac_addr.b[0];
    hash_entry[3] = xi_mac_params_ptr->mac_addr.b[1];
    hash_entry[4] = xi_mac_params_ptr->mac_addr.b[2];
    hash_entry[5] = xi_mac_params_ptr->mac_addr.b[3];
    hash_entry[6] = xi_mac_params_ptr->mac_addr.b[4];
    hash_entry[7] = xi_mac_params_ptr->mac_addr.b[5];

    context_entry[0] = xi_mac_params_ptr->bridge_port;
    context_entry[1] = (xi_mac_params_ptr->sa_action << 0) | (xi_mac_params_ptr->da_action << 3);

    bdmf_fastlock_lock_irq(&int_lock_irq, flags);

    /* check if entry exists, than manipulate it according to type (static/bridge) */
    rdd_error = rdd_find_entry_64_bit(&g_hash_table_cfg[BL_LILAC_RDD_MAC_TABLE],
                                      hash_entry,
                                      MAC_ENTRY_KEY_MASK_HIGH,
                                      MAC_ENTRY_KEY_MASK_LOW,
                                      0,
                                      xo_mac_entry_index);

    if (rdd_error == BL_LILAC_RDD_OK)
    {
        rdd_error = p_rdd_add_mac_entry_type_handle(&(xi_mac_params_ptr->mac_addr),
                                                    xi_mac_params_ptr->bridge_port,
                                                    xi_mac_params_ptr->aggregation_mode,
                                                    xi_mac_params_ptr->extension_entry,
                                                    xi_mac_params_ptr->sa_action,
                                                    xi_mac_params_ptr->da_action,
                                                    *xo_mac_entry_index);

        bdmf_fastlock_unlock_irq(&int_lock_irq, flags);
        return (rdd_error);
    }

    hash_entry[0] = (xi_mac_params_ptr->bridge_port >> 4) & 0x01;
    hash_entry[1] = (xi_mac_params_ptr->bridge_port << 4) & 0xF0;

    /* add entry to table */
    rdd_error = rdd_add_hash_entry_64_bit(&g_hash_table_cfg[BL_LILAC_RDD_MAC_TABLE],
                                          hash_entry,
                                          context_entry,
                                          MAC_ENTRY_KEY_MASK_HIGH,
                                          MAC_ENTRY_KEY_MASK_LOW,
                                          0,
                                          xo_mac_entry_index);

    if (rdd_error)
    {
        bdmf_fastlock_unlock_irq(&int_lock_irq, flags);
        return (rdd_error);
    }

    /* if added entry is static, mark corresponding bit in entry */
    if (xi_mac_params_ptr->bridge_port == BL_LILAC_RDD_CPU_BRIDGE_PORT)
    {
        f_rdd_mac_entry_set_type_bit(*xo_mac_entry_index, BL_LILAC_RDD_STATIC_MAC_ADDRESS);

        g_static_mac_counter[*xo_mac_entry_index]++;
    }
    else
    {
        f_rdd_mac_entry_set_type_bit(*xo_mac_entry_index, BL_LILAC_RDD_BRIDGE_MAC_ADDRESS);
    }

    /* set extension entry (ssid or aggregation vid) */
    if ((xi_mac_params_ptr->bridge_port != BL_LILAC_RDD_PCI_BRIDGE_PORT) && !(xi_mac_params_ptr->bridge_port >= BL_LILAC_RDD_MULTICAST_LAN0_BRIDGE_PORT))
    {
        xi_mac_params_ptr->extension_entry |= (xi_mac_params_ptr->aggregation_mode << 7);
    }

    rdd_error = p_rdd_write_mac_extension_entry(*xo_mac_entry_index, xi_mac_params_ptr->extension_entry);

    bdmf_fastlock_unlock_irq(&int_lock_irq, flags);
    return (BL_LILAC_RDD_OK);
}

BL_LILAC_RDD_ERROR_DTE rdd_mac_entry_delete(bdmf_mac_t *xi_mac_addr,
                                            BL_LILAC_RDD_MAC_ENTRY_TYPE_DTE xi_entry_type)
{
    BL_LILAC_RDD_ERROR_DTE rdd_error;
    RDD_MAC_EXTENSION_TABLE_DTS *mac_extension_table_ptr;
    RDD_MAC_EXTENSION_ENTRY_DTS *mac_extension_entry_ptr;
    uint8_t hash_entry[8];
    uint8_t extension_entry;
    uint32_t new_entry_index;
    uint32_t entry_index;
    uint32_t type_handle_status;
    unsigned long flags;

    hash_entry[0] = 0;
    hash_entry[1] = 0;
    hash_entry[2] = xi_mac_addr->b[0];
    hash_entry[3] = xi_mac_addr->b[1];
    hash_entry[4] = xi_mac_addr->b[2];
    hash_entry[5] = xi_mac_addr->b[3];
    hash_entry[6] = xi_mac_addr->b[4];
    hash_entry[7] = xi_mac_addr->b[5];

    bdmf_fastlock_lock_irq(&int_lock_irq, flags);

    /* get index of deleted entry */
    rdd_error = rdd_find_entry_64_bit(&g_hash_table_cfg[BL_LILAC_RDD_MAC_TABLE],
                                      hash_entry,
                                      MAC_ENTRY_KEY_MASK_HIGH,
                                      MAC_ENTRY_KEY_MASK_LOW,
                                      0,
                                      &new_entry_index);

    if (rdd_error != BL_LILAC_RDD_OK)
    {
        bdmf_fastlock_unlock_irq(&int_lock_irq, flags);
        return BL_LILAC_RDD_ERROR_MAC_ENTRY_DOESNT_EXIST;
    }

    /* perform logic for static/bridge entry */
    rdd_error = p_rdd_delete_mac_entry_type_handle(xi_mac_addr, new_entry_index, xi_entry_type, &type_handle_status);

    if (type_handle_status == LILAC_RDD_TRUE)
    {
        bdmf_fastlock_unlock_irq(&int_lock_irq, flags);
        return rdd_error;
    }

    rdd_error = rdd_remove_hash_entry_64_bit(&g_hash_table_cfg[BL_LILAC_RDD_MAC_TABLE],
                                             hash_entry,
                                             MAC_ENTRY_KEY_MASK_HIGH,
                                             MAC_ENTRY_KEY_MASK_LOW,
                                             0,
                                             BL_LILAC_RDD_CAM_OPTIMIZATION_DISABLE,
                                             &entry_index);

    /* if deleted entry was in CAM, set extension entry of last entry to the deleted slot */
    if ((rdd_error == BL_LILAC_RDD_OK) && (entry_index >= g_mac_table_size) && (entry_index != new_entry_index))
    {
        mac_extension_table_ptr = (RDD_MAC_EXTENSION_TABLE_DTS *)(DEVICE_ADDRESS(RUNNER_COMMON_0_OFFSET) + MAC_EXTENSION_TABLE_CAM_ADDRESS);

        /* get extension entry from last entry */
        mac_extension_entry_ptr = &(mac_extension_table_ptr->entry[entry_index % g_mac_table_size]);

        RDD_MAC_EXTENSION_ENTRY_EXTENSION_ENTRY_READ(extension_entry, mac_extension_entry_ptr);
        RDD_MAC_EXTENSION_ENTRY_EXTENSION_ENTRY_WRITE(0, mac_extension_entry_ptr);

        /* set extension entry to optimized entry */
        mac_extension_entry_ptr = &(mac_extension_table_ptr->entry[new_entry_index % g_mac_table_size]);

        RDD_MAC_EXTENSION_ENTRY_EXTENSION_ENTRY_WRITE(extension_entry, mac_extension_entry_ptr);

        g_static_mac_counter[new_entry_index] = g_static_mac_counter[entry_index];
    }

    g_static_mac_counter[entry_index] = 0;

    bdmf_fastlock_unlock_irq(&int_lock_irq, flags);
    return BL_LILAC_RDD_OK;
}

BL_LILAC_RDD_ERROR_DTE rdd_mac_entry_modify ( RDD_MAC_PARAMS  *xi_mac_params_ptr )
{
    BL_LILAC_RDD_ERROR_DTE  rdd_error;
    unsigned long flags;

    bdmf_fastlock_lock_irq(&int_lock_irq, flags);

    rdd_error = f_rdd_mac_entry_modify ( xi_mac_params_ptr );

    bdmf_fastlock_unlock_irq(&int_lock_irq, flags);
    return (rdd_error);
}

BL_LILAC_RDD_ERROR_DTE rdd_mac_entry_search ( RDD_MAC_PARAMS  *xi_mac_params_ptr,
                                              uint32_t        *xo_entry_index )
{
    BL_LILAC_RDD_ERROR_DTE  rdd_error;
    uint8_t                 hash_entry[8];

    hash_entry[0] = 0;
    hash_entry[1] = 0;
    hash_entry[2] = xi_mac_params_ptr->mac_addr.b[0];
    hash_entry[3] = xi_mac_params_ptr->mac_addr.b[1];
    hash_entry[4] = xi_mac_params_ptr->mac_addr.b[2];
    hash_entry[5] = xi_mac_params_ptr->mac_addr.b[3];
    hash_entry[6] = xi_mac_params_ptr->mac_addr.b[4];
    hash_entry[7] = xi_mac_params_ptr->mac_addr.b[5];

    rdd_error = rdd_find_entry_64_bit ( &g_hash_table_cfg[BL_LILAC_RDD_MAC_TABLE],
                                        hash_entry,
                                        MAC_ENTRY_KEY_MASK_HIGH,
                                        MAC_ENTRY_KEY_MASK_LOW,
                                        0,
                                        xo_entry_index );

    if ( rdd_error )
    {
        return ( BL_LILAC_RDD_ERROR_GET_MAC_ENTRY );
    }

    return (BL_LILAC_RDD_OK);
}

BL_LILAC_RDD_ERROR_DTE rdd_mac_entry_get ( uint32_t        xi_entry_index,
                                           RDD_MAC_PARAMS  *xo_mac_params_ptr,
                                           uint32_t        *xo_valid_bit,
                                           uint32_t        *xo_skip_bit,
                                           uint32_t        *xo_aging_bit )
{
    RDD_MAC_TABLE_DTS            *mac_table_ptr;
    RDD_MAC_ENTRY_DTS            *mac_entry_ptr;
    RDD_MAC_EXTENSION_TABLE_DTS  *mac_extension_table_ptr;
    RDD_MAC_EXTENSION_ENTRY_DTS  *mac_extension_entry_ptr;
    RDD_MAC_CONTEXT_TABLE_DTS    *forward_table_ptr;
    RDD_MAC_CONTEXT_ENTRY_DTS    *forward_entry_ptr;
    uint32_t                     forward_entry_multicast;
    uint32_t                     multicast_vector_msb;

    /* check the validity of the input parameters - MAC entry index */
    if ( xi_entry_index >= ( g_mac_table_size + RDD_MAC_TABLE_CAM_SIZE ) )
    {
        return ( BL_LILAC_RDD_ERROR_ILLEGAL_MAC_ENTRY_ID );
    }

    if ( xi_entry_index < g_mac_table_size )
    {
        mac_table_ptr = ( RDD_MAC_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + MAC_TABLE_ADDRESS );

        forward_table_ptr = ( RDD_MAC_CONTEXT_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + MAC_CONTEXT_TABLE_ADDRESS );

        mac_extension_table_ptr = ( RDD_MAC_EXTENSION_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + MAC_EXTENSION_TABLE_ADDRESS );
    }
    else
    {
        mac_table_ptr = ( RDD_MAC_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + MAC_TABLE_CAM_ADDRESS );

        forward_table_ptr = ( RDD_MAC_CONTEXT_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + MAC_CONTEXT_TABLE_CAM_ADDRESS );

        mac_extension_table_ptr = ( RDD_MAC_EXTENSION_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + MAC_EXTENSION_TABLE_CAM_ADDRESS );

        xi_entry_index %= g_mac_table_size;
    }

    /* read MAC table entry */
    mac_entry_ptr = &( mac_table_ptr->entry[xi_entry_index] );

    /* retreive the MAC entry information - MAC address */
    RDD_MAC_ENTRY_MAC_ADDR0_READ ( xo_mac_params_ptr->mac_addr.b[0], mac_entry_ptr );
    RDD_MAC_ENTRY_MAC_ADDR1_READ ( xo_mac_params_ptr->mac_addr.b[1], mac_entry_ptr );
    RDD_MAC_ENTRY_MAC_ADDR2_READ ( xo_mac_params_ptr->mac_addr.b[2], mac_entry_ptr );
    RDD_MAC_ENTRY_MAC_ADDR3_READ ( xo_mac_params_ptr->mac_addr.b[3], mac_entry_ptr );
    RDD_MAC_ENTRY_MAC_ADDR4_READ ( xo_mac_params_ptr->mac_addr.b[4], mac_entry_ptr );
    RDD_MAC_ENTRY_MAC_ADDR5_READ ( xo_mac_params_ptr->mac_addr.b[5], mac_entry_ptr );

    /* retrieve forwarding information */
    forward_entry_ptr = &( forward_table_ptr->entry[xi_entry_index] );

    RDD_MAC_CONTEXT_ENTRY_EGRESS_PORT_READ ( xo_mac_params_ptr->bridge_port, forward_entry_ptr );
    RDD_MAC_CONTEXT_ENTRY_SA_ACTION_READ ( xo_mac_params_ptr->sa_action, forward_entry_ptr );
    RDD_MAC_CONTEXT_ENTRY_DA_ACTION_READ ( xo_mac_params_ptr->da_action, forward_entry_ptr );
    RDD_MAC_CONTEXT_ENTRY_MULTICAST_READ ( forward_entry_multicast, forward_entry_ptr );

    if ( forward_entry_multicast )
    {
        /* multicast egress ports vector consists of two fields in forward entry */
        RDD_MAC_CONTEXT_ENTRY_MULTICAST_VECTOR_READ ( multicast_vector_msb, forward_entry_ptr );

        xo_mac_params_ptr->bridge_port = ( xo_mac_params_ptr->bridge_port | ( multicast_vector_msb << 5 ) ) << 4;
    }

    RDD_MAC_CONTEXT_ENTRY_MAC_TYPE_READ ( xo_mac_params_ptr->entry_type, forward_entry_ptr );

    mac_extension_entry_ptr = &( mac_extension_table_ptr->entry[xi_entry_index] );

    RDD_MAC_EXTENSION_ENTRY_EXTENSION_ENTRY_READ ( xo_mac_params_ptr->extension_entry, mac_extension_entry_ptr );

    if ( xo_mac_params_ptr->extension_entry & ( 1 << 7 ) )
    {
        xo_mac_params_ptr->extension_entry &= ~( 1 << 7 );
        xo_mac_params_ptr->aggregation_mode = BL_LILAC_RDD_AGGREGATION_MODE_ENABLED;
    }
    else
    {
        xo_mac_params_ptr->aggregation_mode = BL_LILAC_RDD_AGGREGATION_MODE_DISABLED;
    }

    /* retreive the entry's control bits */
    RDD_MAC_ENTRY_VALID_READ ( *xo_valid_bit, mac_entry_ptr );
    RDD_MAC_ENTRY_SKIP_READ ( *xo_skip_bit, mac_entry_ptr );
    RDD_MAC_ENTRY_AGING_READ ( *xo_aging_bit, mac_entry_ptr );

    return (BL_LILAC_RDD_OK);
}

BL_LILAC_RDD_ERROR_DTE rdd_mac_entry_aging_set ( uint32_t  xi_entry_index,
                                                 uint32_t  *xo_activity_status )
{
    return (BL_LILAC_RDD_OK);
}

BL_LILAC_RDD_ERROR_DTE rdd_clear_mac_table ( void )
{
    return (BL_LILAC_RDD_OK);
}

BL_LILAC_RDD_ERROR_DTE rdd_forwarding_matrix_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE     xi_src_bridge_port,
                                                      BL_LILAC_RDD_BRIDGE_PORT_DTE     xi_dst_bridge_port,
                                                      BL_LILAC_RDD_FORWARD_MATRIX_DTE  xi_enable )
{
    return (BL_LILAC_RDD_OK);
}

BL_LILAC_RDD_ERROR_DTE rdd_wifi_ssid_forwarding_matrix_config ( uint16_t                      xi_wifi_ssid_vector,
                                                                BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_dst_bridge_port )
{
    return (BL_LILAC_RDD_OK);
}

#if defined(LEGACY_RDP)
BL_LILAC_RDD_ERROR_DTE rdd_egress_ethertype_config ( uint16_t  xi_1st_ether_type,
                                                     uint16_t  xi_2nd_ether_type )
{
    return (BL_LILAC_RDD_OK);
}
#else
void rdd_egress_ethertype_config ( uint16_t  xi_1st_ether_type,
                                   uint16_t  xi_2nd_ether_type )
{
}
#endif

BL_LILAC_RDD_ERROR_DTE rdd_src_mac_anti_spoofing_lookup_config(BL_LILAC_RDD_BRIDGE_PORT_DTE xi_bridge_port,
                                                               BL_LILAC_RDD_FILTER_MODE_DTE xi_filter_mode)
{
    RDD_US_INGRESS_FILTERS_CONFIGURATION_TABLE_DTS *us_filters_cfg_table_ptr;
    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DTS *filters_cfg_entry_ptr;
    int32_t bridge_port_index;

    /* TODO!! do we even support SRC MAC anti-spoofing? if not, we can choose to not configure this filter */
    us_filters_cfg_table_ptr = RDD_US_INGRESS_FILTERS_CONFIGURATION_TABLE_PTR();

    if ((bridge_port_index = rdd_bridge_port_to_port_index(xi_bridge_port, 0)) < 0)
        return (BL_LILAC_RDD_ERROR_ILLEGAL_BRIDGE_PORT_ID);

    filters_cfg_entry_ptr = &(us_filters_cfg_table_ptr->entry[bridge_port_index]);

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_SRC_MAC_ANTI_SPOOFING_LOOKUP_WRITE(xi_filter_mode, filters_cfg_entry_ptr);

    return (BL_LILAC_RDD_OK);
}


BL_LILAC_RDD_ERROR_DTE rdd_src_mac_anti_spoofing_entry_add(BL_LILAC_RDD_BRIDGE_PORT_DTE xi_bridge_port,
                                                           uint32_t xi_src_mac_prefix)
{
    return (BL_LILAC_RDD_OK);
}

BL_LILAC_RDD_ERROR_DTE rdd_src_mac_anti_spoofing_entry_delete(BL_LILAC_RDD_BRIDGE_PORT_DTE xi_bridge_port,
                                                              uint32_t xi_src_mac_prefix)
{
    return (BL_LILAC_RDD_OK);
}

BL_LILAC_RDD_ERROR_DTE rdd_tpid_overwrite_table_config ( uint16_t          *tpid_overwrite_array,
                                                         rdpa_traffic_dir  xi_direction )
{
    return (BL_LILAC_RDD_OK);
}

#if defined(LEGACY_RDP)
BL_LILAC_RDD_ERROR_DTE rdd_vlan_command_always_egress_ether_type_config ( uint16_t  xi_3rd_ether_type )
{
    return (BL_LILAC_RDD_OK);
}

BL_LILAC_RDD_ERROR_DTE rdd_vlan_switching_isolation_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_port,
                                                             rdpa_traffic_dir              xi_direction,
                                                             BL_LILAC_RDD_FILTER_MODE_DTE  xi_vlan_switching_isolation_mode )
{
    return (BL_LILAC_RDD_OK);
}
#else
void rdd_vlan_command_always_egress_ether_type_config ( uint16_t  xi_3rd_ether_type )
{
}
#endif

BL_LILAC_RDD_ERROR_DTE rdd_lan_get_stats ( uint32_t   xi_lan_port,
                                           uint32_t   *rx_packet,
                                           uint32_t   *tx_packet,
                                           uint16_t   *tx_discard )
{
    BL_LILAC_RDD_ERROR_DTE rdd_error = BL_LILAC_RDD_OK;

    uint32_t counter_offset = xi_lan_port + 1;  /* skip the first one which is used for WAN */

    rdd_error = rdd_2_bytes_counter_get ( BRIDGE_DOWNSTREAM_TX_CONGESTION_GROUP, counter_offset, tx_discard );
    if (!rdd_error)
        rdd_error = rdd_4_bytes_counter_get ( LAN_TX_PACKETS_GROUP, counter_offset + 8, tx_packet );
    if (!rdd_error)
        rdd_error = rdd_4_bytes_counter_get ( LAN_RX_PACKETS_GROUP, counter_offset + 8, rx_packet );

    return ( rdd_error );
}

BL_LILAC_RDD_ERROR_DTE f_rdd_ingress_filters_cam_initialize(void)
{
    RDD_DS_INGRESS_FILTERS_LOOKUP_TABLE_DTS *ds_filters_cam_table_ptr = NULL;
    RDD_US_INGRESS_FILTERS_LOOKUP_TABLE_DTS *us_filters_cam_table_ptr = NULL;
    RDD_INGRESS_FILTERS_LOOKUP_ENTRY_DTS *filters_cam_entry_ptr;
    BL_LILAC_RDD_BRIDGE_PORT_DTE bridge_port;
    uint32_t subnet_id;
    int32_t bridge_port_index;

    g_ether_type_filter_mode = (BL_LILAC_RDD_ETHER_TYPE_FILTER_MATRIX_DTS *)malloc(sizeof(BL_LILAC_RDD_ETHER_TYPE_FILTER_MATRIX_DTS));

    if (g_ether_type_filter_mode == NULL)
        return (BL_LILAC_RDD_ERROR_MALLOC_FAILED);

    for (bridge_port = BL_LILAC_RDD_WAN_BRIDGE_PORT; bridge_port <= BL_LILAC_RDD_PCI_BRIDGE_PORT; bridge_port++)
    {
        
        if BL_LILAC_RDD_IS_WAN_BRIDGE_PORT(bridge_port)
            ds_filters_cam_table_ptr = RDD_DS_INGRESS_FILTERS_LOOKUP_TABLE_PTR();
        else
            us_filters_cam_table_ptr = RDD_US_INGRESS_FILTERS_LOOKUP_TABLE_PTR();

        for (subnet_id = BL_LILAC_RDD_SUBNET_FLOW_CACHE; subnet_id < LILAC_RDD_NUMBER_OF_SUBNETS; subnet_id++)
        {
            if ((bridge_port_index = rdd_bridge_port_to_port_index(bridge_port, subnet_id)) < 0)
                break;

            if BL_LILAC_RDD_IS_WAN_BRIDGE_PORT(bridge_port)
                filters_cam_entry_ptr = &(ds_filters_cam_table_ptr->entry[bridge_port_index][BL_LILAC_RDD_IGMP_FILTER_NUMBER]);
            else
                filters_cam_entry_ptr = &(us_filters_cam_table_ptr->entry[bridge_port_index - BL_LILAC_RDD_LAN0_BRIDGE_PORT][BL_LILAC_RDD_IGMP_FILTER_NUMBER]);

            f_rdd_ingress_filter_disable(filters_cam_entry_ptr);

            RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_WRITE(PARSER_L4_PROTOCOL_IGMP, filters_cam_entry_ptr);
            RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_MASK_WRITE(0xF, filters_cam_entry_ptr);

            if BL_LILAC_RDD_IS_WAN_BRIDGE_PORT(bridge_port)
                filters_cam_entry_ptr = &(ds_filters_cam_table_ptr->entry[bridge_port_index][BL_LILAC_RDD_ICMPV6_FILTER_NUMBER]);
            else
                filters_cam_entry_ptr = &(us_filters_cam_table_ptr->entry[bridge_port_index - BL_LILAC_RDD_LAN0_BRIDGE_PORT][BL_LILAC_RDD_ICMPV6_FILTER_NUMBER]);

            f_rdd_ingress_filter_disable(filters_cam_entry_ptr);

            RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_WRITE(PARSER_L4_PROTOCOL_ICMPV6, filters_cam_entry_ptr);
            RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_MASK_WRITE(0xF, filters_cam_entry_ptr);

            if BL_LILAC_RDD_IS_WAN_BRIDGE_PORT(bridge_port)
                filters_cam_entry_ptr = &(ds_filters_cam_table_ptr->entry[bridge_port_index][BL_LILAC_RDD_USER_0_FILTER_NUMBER]);
            else
                filters_cam_entry_ptr = &(us_filters_cam_table_ptr->entry[bridge_port_index - BL_LILAC_RDD_LAN0_BRIDGE_PORT][BL_LILAC_RDD_USER_0_FILTER_NUMBER]);

            f_rdd_ingress_filter_disable(filters_cam_entry_ptr);

            RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L2_PROTOCOL_WRITE(PARSER_L2_PROTOCOL_USER_DEFINED_0, filters_cam_entry_ptr);
            RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L2_PROTOCOL_MASK_WRITE(0xF, filters_cam_entry_ptr);

            if BL_LILAC_RDD_IS_WAN_BRIDGE_PORT(bridge_port)
                filters_cam_entry_ptr = &(ds_filters_cam_table_ptr->entry[bridge_port_index][BL_LILAC_RDD_USER_1_FILTER_NUMBER]);
            else
                filters_cam_entry_ptr = &(us_filters_cam_table_ptr->entry[bridge_port_index - BL_LILAC_RDD_LAN0_BRIDGE_PORT][BL_LILAC_RDD_USER_1_FILTER_NUMBER]);

            f_rdd_ingress_filter_disable(filters_cam_entry_ptr);

            RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L2_PROTOCOL_WRITE(PARSER_L2_PROTOCOL_USER_DEFINED_1, filters_cam_entry_ptr);
            RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L2_PROTOCOL_MASK_WRITE(0xF, filters_cam_entry_ptr);

            if BL_LILAC_RDD_IS_WAN_BRIDGE_PORT(bridge_port)
                filters_cam_entry_ptr = &(ds_filters_cam_table_ptr->entry[bridge_port_index][BL_LILAC_RDD_USER_2_FILTER_NUMBER]);
            else
                filters_cam_entry_ptr = &(us_filters_cam_table_ptr->entry[bridge_port_index - BL_LILAC_RDD_LAN0_BRIDGE_PORT][BL_LILAC_RDD_USER_2_FILTER_NUMBER]);

            f_rdd_ingress_filter_disable(filters_cam_entry_ptr);

            RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L2_PROTOCOL_WRITE(PARSER_L2_PROTOCOL_USER_DEFINED_2, filters_cam_entry_ptr);
            RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L2_PROTOCOL_MASK_WRITE(0xF, filters_cam_entry_ptr);

            if BL_LILAC_RDD_IS_WAN_BRIDGE_PORT(bridge_port)
                filters_cam_entry_ptr = &(ds_filters_cam_table_ptr->entry[bridge_port_index][BL_LILAC_RDD_USER_3_FILTER_NUMBER]);
            else
                filters_cam_entry_ptr = &(us_filters_cam_table_ptr->entry[bridge_port_index - BL_LILAC_RDD_LAN0_BRIDGE_PORT][BL_LILAC_RDD_USER_3_FILTER_NUMBER]);

            f_rdd_ingress_filter_disable(filters_cam_entry_ptr);

            RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L2_PROTOCOL_WRITE(PARSER_L2_PROTOCOL_USER_DEFINED_3, filters_cam_entry_ptr);
            RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L2_PROTOCOL_MASK_WRITE(0xF, filters_cam_entry_ptr);

            if BL_LILAC_RDD_IS_WAN_BRIDGE_PORT(bridge_port)
                filters_cam_entry_ptr = &(ds_filters_cam_table_ptr->entry[bridge_port_index][BL_LILAC_RDD_PPPOE_D_FILTER_NUMBER]);
            else
                filters_cam_entry_ptr = &(us_filters_cam_table_ptr->entry[bridge_port_index - BL_LILAC_RDD_LAN0_BRIDGE_PORT][BL_LILAC_RDD_PPPOE_D_FILTER_NUMBER]);

            f_rdd_ingress_filter_disable(filters_cam_entry_ptr);

            RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L2_PROTOCOL_WRITE(PARSER_L2_PROTOCOL_PPPOE_D, filters_cam_entry_ptr);
            RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L2_PROTOCOL_MASK_WRITE(0xF, filters_cam_entry_ptr);

            if BL_LILAC_RDD_IS_WAN_BRIDGE_PORT(bridge_port)
                filters_cam_entry_ptr = &(ds_filters_cam_table_ptr->entry[bridge_port_index][BL_LILAC_RDD_PPPOE_S_FILTER_NUMBER]);
            else
                filters_cam_entry_ptr = &(us_filters_cam_table_ptr->entry[bridge_port_index - BL_LILAC_RDD_LAN0_BRIDGE_PORT][BL_LILAC_RDD_PPPOE_S_FILTER_NUMBER]);

            f_rdd_ingress_filter_disable(filters_cam_entry_ptr);

            RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L2_PROTOCOL_WRITE(PARSER_L2_PROTOCOL_PPPOE_S, filters_cam_entry_ptr);
            RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L2_PROTOCOL_MASK_WRITE(0xF, filters_cam_entry_ptr);

#ifdef UNDEF
            if (bridge_port == BL_LILAC_RDD_WAN_ROUTER_PORT)
            {
                RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L3_PROTOCOL_WRITE(PARSER_L3_PROTOCOL_OTHER, filters_cam_entry_ptr);
                RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L3_PROTOCOL_MASK_WRITE(0x3, filters_cam_entry_ptr);
            }
#endif

            if BL_LILAC_RDD_IS_WAN_BRIDGE_PORT(bridge_port)
                filters_cam_entry_ptr = &(ds_filters_cam_table_ptr->entry[bridge_port_index][BL_LILAC_RDD_ARP_FILTER_NUMBER]);
            else
                filters_cam_entry_ptr = &(us_filters_cam_table_ptr->entry[bridge_port_index - BL_LILAC_RDD_LAN0_BRIDGE_PORT][BL_LILAC_RDD_ARP_FILTER_NUMBER]);

            f_rdd_ingress_filter_disable(filters_cam_entry_ptr);

            RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L2_PROTOCOL_WRITE(PARSER_L2_PROTOCOL_ARP, filters_cam_entry_ptr);
            RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L2_PROTOCOL_MASK_WRITE(0xF, filters_cam_entry_ptr);

            if BL_LILAC_RDD_IS_WAN_BRIDGE_PORT(bridge_port)
                filters_cam_entry_ptr = &(ds_filters_cam_table_ptr->entry[bridge_port_index][BL_LILAC_RDD_1588_FILTER_NUMBER]);
            else
                filters_cam_entry_ptr = &(us_filters_cam_table_ptr->entry[bridge_port_index - BL_LILAC_RDD_LAN0_BRIDGE_PORT][BL_LILAC_RDD_1588_FILTER_NUMBER]);

            f_rdd_ingress_filter_disable(filters_cam_entry_ptr);

            RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L2_PROTOCOL_WRITE(PARSER_L2_PROTOCOL__1588, filters_cam_entry_ptr);
            RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L2_PROTOCOL_MASK_WRITE(0xF, filters_cam_entry_ptr);

            if BL_LILAC_RDD_IS_WAN_BRIDGE_PORT(bridge_port)
                filters_cam_entry_ptr = &(ds_filters_cam_table_ptr->entry[bridge_port_index][BL_LILAC_RDD_802_1X_FILTER_NUMBER]);
            else
                filters_cam_entry_ptr = &(us_filters_cam_table_ptr->entry[bridge_port_index - BL_LILAC_RDD_LAN0_BRIDGE_PORT][BL_LILAC_RDD_802_1X_FILTER_NUMBER]);

            f_rdd_ingress_filter_disable(filters_cam_entry_ptr);

            RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L2_PROTOCOL_WRITE(PARSER_L2_PROTOCOL__802_1X, filters_cam_entry_ptr);
            RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L2_PROTOCOL_MASK_WRITE(0xF, filters_cam_entry_ptr);

            if BL_LILAC_RDD_IS_WAN_BRIDGE_PORT(bridge_port)
                filters_cam_entry_ptr = &(ds_filters_cam_table_ptr->entry[bridge_port_index][BL_LILAC_RDD_802_1AG_CFM_FILTER_NUMBER]);
            else
                filters_cam_entry_ptr = &(us_filters_cam_table_ptr->entry[bridge_port_index - BL_LILAC_RDD_LAN0_BRIDGE_PORT][BL_LILAC_RDD_802_1AG_CFM_FILTER_NUMBER]);

            f_rdd_ingress_filter_disable(filters_cam_entry_ptr);

            RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L2_PROTOCOL_WRITE(PARSER_L2_PROTOCOL__802_1AG_CFM, filters_cam_entry_ptr);
            RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L2_PROTOCOL_MASK_WRITE(0xF, filters_cam_entry_ptr);

            if BL_LILAC_RDD_IS_WAN_BRIDGE_PORT(bridge_port)
                filters_cam_entry_ptr = &(ds_filters_cam_table_ptr->entry[bridge_port_index][BL_LILAC_RDD_BROADCAST_FILTER_NUMBER]);
            else
                filters_cam_entry_ptr = &(us_filters_cam_table_ptr->entry[bridge_port_index - BL_LILAC_RDD_LAN0_BRIDGE_PORT][BL_LILAC_RDD_BROADCAST_FILTER_NUMBER]);

            f_rdd_ingress_filter_disable(filters_cam_entry_ptr);

            RDD_INGRESS_FILTERS_LOOKUP_ENTRY_BROADCAST_WRITE(LILAC_RDD_ON, filters_cam_entry_ptr);
            RDD_INGRESS_FILTERS_LOOKUP_ENTRY_BROADCAST_MASK_WRITE(0x1, filters_cam_entry_ptr);

            if BL_LILAC_RDD_IS_WAN_BRIDGE_PORT(bridge_port)
                filters_cam_entry_ptr = &(ds_filters_cam_table_ptr->entry[bridge_port_index][BL_LILAC_RDD_MULTICAST_FILTER_NUMBER]);
            else
                filters_cam_entry_ptr = &(us_filters_cam_table_ptr->entry[bridge_port_index - BL_LILAC_RDD_LAN0_BRIDGE_PORT][BL_LILAC_RDD_MULTICAST_FILTER_NUMBER]);

            f_rdd_ingress_filter_disable(filters_cam_entry_ptr);

            RDD_INGRESS_FILTERS_LOOKUP_ENTRY_MULTICAST_WRITE(LILAC_RDD_ON, filters_cam_entry_ptr);
            RDD_INGRESS_FILTERS_LOOKUP_ENTRY_MULTICAST_MASK_WRITE(0x1, filters_cam_entry_ptr);

            if BL_LILAC_RDD_IS_WAN_BRIDGE_PORT(bridge_port)
                filters_cam_entry_ptr = &(ds_filters_cam_table_ptr->entry[bridge_port_index][BL_LILAC_RDD_INGRESS_FILTERS_NUMBER]);
            else
                filters_cam_entry_ptr = &(us_filters_cam_table_ptr->entry[bridge_port_index - BL_LILAC_RDD_LAN0_BRIDGE_PORT][BL_LILAC_RDD_INGRESS_FILTERS_NUMBER]);

            MWRITE_16(filters_cam_entry_ptr, 0xFFFF);
        }
    }

    for (bridge_port = BL_LILAC_RDD_WAN_BRIDGE_PORT; bridge_port <= BL_LILAC_RDD_PCI_BRIDGE_PORT; bridge_port++)
        g_local_switching_filters_mode[bridge_port] = BL_LILAC_RDD_FILTER_DISABLE;

    return (BL_LILAC_RDD_OK);
}

#if !defined(LEGACY_RDP)
int rdd_src_mac_anti_spoofing_lookup_cfg(rdd_port_profile_t profile_idx, bdmf_boolean enable)
{
    return rdd_src_mac_anti_spoofing_lookup_config(profile_idx, enable);
}

int rdd_tpid_detect_filter_cfg(rdd_port_profile_t profile_idx, rdpa_traffic_dir dir, bdmf_boolean enable)
{
    return rdd_tpid_detect_filter_config(profile_idx, 0, enable);
}

int rdd_dhcp_filter_cfg(rdd_port_profile_t profile_idx, rdpa_traffic_dir dir, bdmf_boolean enable,
    bdmf_boolean local_switch_en)
{
    return rdd_dhcp_filter_config(profile_idx, 0, enable);
}

int rdd_mld_filter_cfg(rdd_port_profile_t profile_idx, rdpa_traffic_dir dir, bdmf_boolean enable,
    bdmf_boolean local_switch_en)
{
    return rdd_mld_filter_config(profile_idx, 0, enable);
}

int rdd_igmp_filter_cfg(rdd_port_profile_t profile_idx, rdpa_traffic_dir dir,
    bdmf_boolean enable, rdd_action filter_action, bdmf_boolean local_switch_en)
{
    return rdd_igmp_filter_config(profile_idx, 0, enable, filter_action);
}

int rdd_icmpv6_filter_cfg(rdd_port_profile_t profile_idx, rdpa_traffic_dir dir,
    bdmf_boolean enable, bdmf_boolean local_switch_en)
{
    return rdd_icmpv6_filter_config(profile_idx, 0, enable);
}

int rdd_ether_type_filter_cfg(rdd_port_profile_t profile_idx, rdpa_traffic_dir dir, bdmf_boolean enable,
    uint8_t ether_type_filter_num, rdd_action filter_action, bdmf_boolean local_switch_en)
{
    return rdd_ether_type_filter_config(profile_idx, 0, enable, ether_type_filter_num, filter_action);
}

int rdd_bcast_filter_cfg(rdd_port_profile_t profile_idx, rdpa_traffic_dir dir, bdmf_boolean enable,
    rdd_action filter_action, bdmf_boolean local_switch_en)
{
    return rdd_broadcast_filter_config(profile_idx, 0, enable, filter_action);
}

int rdd_mcast_filter_cfg(rdd_port_profile_t profile_idx, rdpa_traffic_dir dir, bdmf_boolean enable,
    rdd_action filter_action, bdmf_boolean local_switch_en)
{
    return rdd_multicast_filter_config(profile_idx, 0, enable, filter_action);
}

int rdd_ip_fragments_ingress_filter_cfg(rdd_port_profile_t profile_idx, rdpa_traffic_dir dir, bdmf_boolean enable,
    rdd_action filter_action)
{
    return rdd_ip_fragments_ingress_filter_config(profile_idx, 0, enable, filter_action);
}

int rdd_hdr_err_ingress_filter_cfg(rdd_port_profile_t profile_idx, rdpa_traffic_dir dir, bdmf_boolean enable,
    rdd_action filter_action)
{
    return rdd_header_error_ingress_filter_config(profile_idx, 0, enable, filter_action);
}
#endif


