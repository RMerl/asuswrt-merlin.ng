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


#ifndef _RDPA_COMMON_H_
#define _RDPA_COMMON_H_

#include "bdmf_dev.h"
#include "bdmf_errno.h"
#include "rdpa_port.h"

/** Direction + index, Underlying structure for rdpa_dir_index aggregate */
typedef struct
{
    rdpa_traffic_dir dir;       /** Traffic direction */
    bdmf_index index;           /** Index */
} rdpa_dir_index_t;

typedef struct
{
    int src;
    int dest;
} int2int_map_t;

static inline int int2int_map(int2int_map_t *map, int src, int last)
{
    for (; map->src != last && map->src != src; map++)
        ;
    return map->dest;
}

static inline int int2int_map_r(int2int_map_t *map, int src, int last)
{
    for (; map->src != last && map->dest != src; map++)
        ;
    return map->src;
}

bdmf_error_t rdpa_obj_get(struct bdmf_object **rdpa_objs, int max_rdpa_objs_num, int index, struct bdmf_object **mo);
int rdpa_dir_index_get_next(rdpa_dir_index_t *dir_index, bdmf_index max_index);

rdpa_if rdpa_port_map_from_hw_port(int hw_port, bdmf_boolean emac_only);

#ifdef BDMF_DRIVER
/*
 * Enum tables for framework CLI access
 */
extern const bdmf_attr_enum_table_t rdpa_if_enum_table;
extern const bdmf_attr_enum_table_t rdpa_lan_wan_if_enum_table;
extern const bdmf_attr_enum_table_t rdpa_lan_wan_wlan_if_enum_table;
extern const bdmf_attr_enum_table_t rdpa_lan_or_cpu_if_enum_table;
extern const bdmf_attr_enum_table_t rdpa_wlan_ssid_enum_table;
extern const bdmf_attr_enum_table_t rdpa_emac_enum_table;
extern const bdmf_attr_enum_table_t rdpa_wan_emac_enum_table;
extern const bdmf_attr_enum_table_t rdpa_wan_type_enum_table;
extern const bdmf_attr_enum_table_t rdpa_forward_action_enum_table;
extern const bdmf_attr_enum_table_t rdpa_filter_action_enum_table;
extern const bdmf_attr_enum_table_t rdpa_traffic_dir_enum_table;
extern const bdmf_attr_enum_table_t rdpa_port_frame_allow_enum_table;
extern const bdmf_attr_enum_table_t rdpa_qos_method_enum_table;
extern const bdmf_attr_enum_table_t rdpa_ip_version_enum_table;
extern const bdmf_attr_enum_table_t rdpa_forward_mode_enum_table;
extern const bdmf_attr_enum_table_t rdpa_classify_mode_enum_table;
extern const bdmf_attr_enum_table_t rdpa_disc_prty_enum_table;
extern const bdmf_attr_enum_table_t rdpa_flow_dest_enum_table;
extern const bdmf_attr_enum_table_t rdpa_ip_class_method_enum_table;
extern const bdmf_attr_enum_table_t rdpa_cpu_reason_enum_table;
extern const bdmf_attr_enum_table_t rdpa_epon_mode_enum_table;
extern const bdmf_attr_enum_table_t rdpa_ic_act_vect_enum_table;
extern const bdmf_attr_enum_table_t rdpa_ic_dei_command_enum_table;
extern const bdmf_attr_enum_table_t rdpa_bpm_buffer_size_enum_table;
extern const bdmf_attr_enum_table_t rdpa_wl_accel_enum_table;
#endif /* #ifdef BDMF_DRIVER */

#endif
