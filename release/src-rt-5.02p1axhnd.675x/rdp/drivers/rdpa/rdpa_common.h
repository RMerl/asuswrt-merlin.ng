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

#include "rdpa_types.h"
#include "bdmf_dev.h"
#include "bdmf_errno.h"
#include "rdpa_port.h"
#include "rdpa_flow_idx_pool.h"

#define SPRINTF_CHECK_RET(ret, buf, avail_size, str_len, args...)   \
    do                                                              \
    {                                                               \
        str_len = snprintf(buf, avail_size, ##args);                \
        buf += str_len;                                             \
        avail_size -= str_len;                                      \
        if (!avail_size)                                            \
            return ret;                                             \
    } while (0)



int rdpa_wl_metadata_val_to_s(uint32_t wl_meta, char *sbuf, uint32_t size, int is_mcast);
bdmf_error_t rdpa_obj_get(struct bdmf_object **rdpa_objs, int max_rdpa_objs_num, int index, struct bdmf_object **mo);
int rdpa_dir_index_get_next(rdpa_dir_index_t *dir_index, bdmf_index max_index);
void replace_ownership(bdmf_object_handle current_mo, bdmf_object_handle new_mo, bdmf_object_handle owner);
int rdpa_flow_get_next(rdpa_flow_idx_pool_t *pool_p, bdmf_index *index, rdpa_flow_t type);
int rdpa_flow_get_ids(rdpa_flow_idx_pool_t *pool_p, uint32_t rdpa_flow_idx, 
                      uint32_t *rdpa_flow_id, uint32_t *rdd_flow_id, rdpa_flow_t type);

#if defined(XRDP)
void rdpa_common_update_cntr_results_uint32(void *stat_buf, void *accumulative_stat_buf, uint32_t stat_offset_in_bytes, uint32_t cntr_result);

typedef enum
{
    rdpa_stat_pckts_id,
    rdpa_stat_bytes_id
} rdpa_stat_elements_ids_t;

static inline uint32_t _get_rdpa_stat_offset(rdpa_stat_elements_ids_t id)
{
    uint32_t offset_in_bytes[rdpa_stat_bytes_id + 1] =
    {
        offsetof(rdpa_stat_t, packets),
        offsetof(rdpa_stat_t, bytes)
    };

    return offset_in_bytes[id];
}
#endif

typedef struct queue_id_info
{
    int rc_id;                                        /* rdd rate controller index */
    int queue;                                        /* physical queue in  RDD  */
    int channel;                                      /* physical channel in RDD   */
} queue_info_t;

typedef enum
{
    rdpa_reserved_option_0,            /* bit 0 is reserved */
    rdpa_ecn_ipv6_remarking_option,    /* bit 1 set ECN IPV6 option*/
    rdpa_g9991_single_fragment_option, /* bit 2 set G9991 single fragment option*/
    rdpa_cpu_mirroring_option          /* bit 3 enable cpu mirroring option*/
} rdpa_system_options_types;

typedef enum
{
    rdpa_anti_spoofing_bypass_option,
} rdpa_port_vlan_options_types;

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
extern const bdmf_attr_enum_table_t rdpa_ic_trap_reason_enum_table;
extern const bdmf_attr_enum_table_t rdpa_filter_enum_table;
extern const bdmf_attr_enum_table_t rdpa_speed_type_enum_table;
extern const bdmf_attr_enum_table_t rdpa_protocol_filters_table;
extern const bdmf_attr_enum_table_t rdpa_tc_enum_table;
extern const bdmf_attr_enum_table_t rdpa_l2_flow_key_exclude_enum_table;
extern const bdmf_attr_enum_table_t rdpa_stat_type_enum_table;
#endif /* #ifdef BDMF_DRIVER */

#endif
