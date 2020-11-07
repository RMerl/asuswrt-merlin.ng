/*
 * <:copyright-BRCM:2015:proprietary:standard
 * 
 *    Copyright (c) 2015 Broadcom 
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

#include "rdpa_api.h"
#ifdef LEGACY_RDP
#include "rdpa_rdd_map_legacy.h"
#else
#include "rdpa_rdd_map.h"
#endif
#include "rdpa_iptv_ex.h"
#include "rdpa_rdd_inline.h"
#include "rdp_drv_ih.h"
#include "rdd_ih_defs.h"
#include "rdp_drv_bbh.h"

extern struct bdmf_object *iptv_object;
extern uint32_t accumulative_iptv_filter_counter;

/** This optional callback is called called at object init time
 *  before initial attributes are set.
 *  If function returns error code !=0, object creation is aborted
 */

extern void rdpa_iptv_channel_key2rdd_iptv_entry_key(const rdpa_iptv_channel_key_t *key, rdpa_ports ports,
    uint16_t wlan_mcast_index, rdd_iptv_entry_t *rdd_iptv_entry);
extern bdmf_error_t rdpa_iptv_channel_rdd_get(bdmf_index channel_index, rdpa_iptv_channel_key_t *key,
    rdpa_ports *ports, uint16_t *wlan_mcast_index, mcast_result_entry_t **entry);

void _rdpa_ih_cfg_iptv_lookup_classifier(void)
{
    DRV_IH_CLASSIFIER_CONFIG ih_classifier_config;

    /* Configure the ih IGMP classifier */
    memset(&ih_classifier_config, 0, sizeof(DRV_IH_CLASSIFIER_CONFIG));

    ih_classifier_config.mask                    = RDPA_MASK_IH_CLASS_KEY_L4;
    ih_classifier_config.resulting_class         = DRV_RDD_IH_CLASS_WAN_BRIDGED_LOW_INDEX;
    ih_classifier_config.l4_protocol             = DRV_IH_L4_PROTOCOL_IGMP;
    /* Configure a classifier for broadcast traffic arriving on IPTV flow */
    fi_bl_drv_ih_configure_classifier(RDPA_IH_CLASSIFIER_IGMP_IPTV, &ih_classifier_config);

    /* Configure the ih ICMPV6 classifier */
    memset(&ih_classifier_config, 0, sizeof(DRV_IH_CLASSIFIER_CONFIG));

    ih_classifier_config.mask                    = RDPA_MASK_IH_CLASS_KEY_L4;
    ih_classifier_config.resulting_class         = DRV_RDD_IH_CLASS_WAN_BRIDGED_LOW_INDEX;
    ih_classifier_config.l4_protocol             = DRV_IH_L4_PROTOCOL_ICMPV6;
    /* Configure a classifier for broadcast traffic arriving on IPTV flow */
    fi_bl_drv_ih_configure_classifier(RDPA_IH_CLASSIFIER_ICMPV6, &ih_classifier_config);
}


static int _rdpa_iptv_ds_ic_classification_lookup_table_config(void)
{
    DRV_IH_LOOKUP_TABLE_60_BIT_KEY_CONFIG lookup_table_60_bit_key_config;

    /*** table 3: DS ingress classification ***/
    lookup_table_60_bit_key_config.table_base_address_in_8_byte = MS_BYTE_TO_8_BYTE_RESOLUTION(
        DRV_RDD_IH_LOOKUP_TABLE_3_BASE_ADDRESS);
    lookup_table_60_bit_key_config.table_size = DRV_RDD_IH_LOOKUP_TABLE_3_SIZE;
    lookup_table_60_bit_key_config.maximal_search_depth = DRV_RDD_IH_LOOKUP_TABLE_3_SEARCH_DEPTH;
    lookup_table_60_bit_key_config.hash_type = DRV_RDD_IH_LOOKUP_TABLE_3_HASH_TYPE;
    lookup_table_60_bit_key_config.sa_search_enable = DRV_RDD_IH_LOOKUP_TABLE_3_SA_ENABLE;
    lookup_table_60_bit_key_config.aging_enable = DRV_RDD_IH_LOOKUP_TABLE_3_AGING_ENABLE;
    lookup_table_60_bit_key_config.cam_enable = DRV_RDD_IH_LOOKUP_TABLE_3_CAM_ENABLE;
    lookup_table_60_bit_key_config.cam_base_address_in_8_byte = MS_BYTE_TO_8_BYTE_RESOLUTION(
        DRV_RDD_IH_LOOKUP_TABLE_3_CAM_BASE_ADDRESS);
    lookup_table_60_bit_key_config.context_table_base_address_in_8_byte = MS_BYTE_TO_8_BYTE_RESOLUTION(
        DRV_RDD_IH_CONTEXT_TABLE_3_BASE_ADDRESS);
    lookup_table_60_bit_key_config.context_table_entry_size = DRV_RDD_IH_CONTEXT_TABLE_3_ENTRY_SIZE;
    lookup_table_60_bit_key_config.cam_context_base_address_in_8_byte = MS_BYTE_TO_8_BYTE_RESOLUTION(
        DRV_RDD_IH_CONTEXT_TABLE_3_CAM_BASE_ADDRESS);
    lookup_table_60_bit_key_config.part_0_start_offset_in_4_byte = DRV_RDD_IH_LOOKUP_TABLE_3_VID_KEY_PART_0_START_OFFSET;
    lookup_table_60_bit_key_config.part_0_shift_offset_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_3_VID_KEY_PART_0_SHIFT;
    lookup_table_60_bit_key_config.part_1_start_offset_in_4_byte = DRV_RDD_IH_LOOKUP_TABLE_3_VID_KEY_PART_1_START_OFFSET;
    lookup_table_60_bit_key_config.part_1_shift_offset_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_3_VID_KEY_PART_1_SHIFT;
    lookup_table_60_bit_key_config.key_extension = DRV_RDD_IH_LOOKUP_TABLE_3_VID_KEY_KEY_EXTENSION;
    lookup_table_60_bit_key_config.part_0_mask_low = DRV_RDD_IH_LOOKUP_TABLE_3_VID_KEY_PART_0_MASK_LOW;
    lookup_table_60_bit_key_config.part_0_mask_high = DRV_RDD_IH_LOOKUP_TABLE_3_VID_KEY_PART_0_MASK_HIGH;
    lookup_table_60_bit_key_config.part_1_mask_low = DRV_RDD_IH_LOOKUP_TABLE_3_VID_KEY_PART_1_MASK_LOW;
    lookup_table_60_bit_key_config.part_1_mask_high = DRV_RDD_IH_LOOKUP_TABLE_3_VID_KEY_PART_1_MASK_HIGH;
    lookup_table_60_bit_key_config.global_mask_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_3_VID_KEY_GLOBAL_MASK;

    return fi_bl_drv_ih_configure_lut_60_bit_key(DRV_RDD_IH_LOOKUP_TABLE_DS_INGRESS_CLASSIFICATION_INDEX,
        &lookup_table_60_bit_key_config);
}

int _rdpa_iptv_group_src_ip_lookup_table_config(void)
{
    DRV_IH_LOOKUP_TABLE_60_BIT_KEY_CONFIG lookup_table_60_bit_key_config;

    /*** table 6: IPTV source IP ***/
    lookup_table_60_bit_key_config.table_base_address_in_8_byte = MS_BYTE_TO_8_BYTE_RESOLUTION(
        DRV_RDD_IH_LOOKUP_TABLE_6_BASE_ADDRESS);
    lookup_table_60_bit_key_config.table_size = DRV_RDD_IH_LOOKUP_TABLE_6_SIZE;
    lookup_table_60_bit_key_config.maximal_search_depth = DRV_RDD_IH_LOOKUP_TABLE_6_SEARCH_DEPTH;
    lookup_table_60_bit_key_config.hash_type = DRV_RDD_IH_LOOKUP_TABLE_6_HASH_TYPE;
    lookup_table_60_bit_key_config.sa_search_enable = DRV_RDD_IH_LOOKUP_TABLE_6_SA_ENABLE;
    lookup_table_60_bit_key_config.aging_enable = DRV_RDD_IH_LOOKUP_TABLE_6_AGING_ENABLE;
    lookup_table_60_bit_key_config.cam_enable = DRV_RDD_IH_LOOKUP_TABLE_6_CAM_ENABLE;
    lookup_table_60_bit_key_config.cam_base_address_in_8_byte = MS_BYTE_TO_8_BYTE_RESOLUTION(
        DRV_RDD_IH_LOOKUP_TABLE_6_CAM_BASE_ADDRESS);
    lookup_table_60_bit_key_config.context_table_base_address_in_8_byte = MS_BYTE_TO_8_BYTE_RESOLUTION(
        DRV_RDD_IH_CONTEXT_TABLE_6_BASE_ADDRESS);
    lookup_table_60_bit_key_config.context_table_entry_size = DRV_RDD_IH_CONTEXT_TABLE_6_ENTRY_SIZE;
    lookup_table_60_bit_key_config.cam_context_base_address_in_8_byte = MS_BYTE_TO_8_BYTE_RESOLUTION(
        DRV_RDD_IH_CONTEXT_TABLE_6_CAM_BASE_ADDRESS);
    lookup_table_60_bit_key_config.part_0_start_offset_in_4_byte = DRV_RDD_IH_LOOKUP_TABLE_6_KEY_PART_0_START_OFFSET;
    lookup_table_60_bit_key_config.part_0_shift_offset_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_6_KEY_PART_0_SHIFT;
    lookup_table_60_bit_key_config.part_1_start_offset_in_4_byte = DRV_RDD_IH_LOOKUP_TABLE_6_KEY_PART_1_START_OFFSET;
    lookup_table_60_bit_key_config.part_1_shift_offset_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_6_KEY_PART_1_SHIFT;
    lookup_table_60_bit_key_config.key_extension = DRV_RDD_IH_LOOKUP_TABLE_6_KEY_KEY_EXTENSION;
    lookup_table_60_bit_key_config.part_0_mask_low = DRV_RDD_IH_LOOKUP_TABLE_6_KEY_PART_0_MASK_LOW;
    lookup_table_60_bit_key_config.part_0_mask_high = DRV_RDD_IH_LOOKUP_TABLE_6_KEY_PART_0_MASK_HIGH;
    lookup_table_60_bit_key_config.part_1_mask_low = DRV_RDD_IH_LOOKUP_TABLE_6_KEY_PART_1_MASK_LOW;
    lookup_table_60_bit_key_config.part_1_mask_high = DRV_RDD_IH_LOOKUP_TABLE_6_KEY_PART_1_MASK_HIGH;
    lookup_table_60_bit_key_config.global_mask_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_6_KEY_GLOBAL_MASK;

    return fi_bl_drv_ih_configure_lut_60_bit_key(DRV_RDD_IH_LOOKUP_TABLE_IPTV_SRC_IP_INDEX,
        &lookup_table_60_bit_key_config);
}

static int _rdpa_iptv_ih_class_config(void)
{
    DRV_IH_CLASS_CONFIG class_config;

    class_config.class_search_1 = DRV_RDD_IH_CLASS_1_CLASS_SEARCH_1;
    class_config.class_search_2 = DRV_IH_CLASS_SEARCH_DISABLED;
    class_config.class_search_3 = DRV_RDD_IH_CLASS_1_CLASS_SEARCH_3;
    class_config.class_search_4 = DRV_RDD_IH_CLASS_1_CLASS_SEARCH_4;
    class_config.destination_port_extraction = DRV_RDD_IH_CLASS_1_DESTINATION_PORT_EXTRACTION;
    class_config.direct_mode_default = DRV_RDD_IH_CLASS_1_DIRECT_MODE_DEFAULT;
    class_config.direct_mode_override = DRV_RDD_IH_CLASS_1_DIRECT_MODE_OVERRIDE;
    class_config.drop_on_miss = DRV_IH_OPERATION_BASED_ON_CLASS_SEARCH_BASED_ON_SEARCH1;
    class_config.dscp_to_tci_table_index = DRV_RDD_IH_CLASS_1_DSCP_TO_PBITS_TABLE_INDEX;
    class_config.target_memory_default = DRV_RDD_IH_CLASS_1_TARGET_MEMORY_DEFAULT;
    class_config.target_memory_override = DRV_RDD_IH_CLASS_1_TARGET_MEMORY_OVERRIDE;
    class_config.ingress_qos_default = DRV_RDD_IH_CLASS_1_INGRESS_QOS_DEFAULT;
    class_config.ingress_qos_override = DRV_RDD_IH_CLASS_1_INGRESS_QOS_OVERRIDE;
    class_config.target_runner_default = DRV_RDD_IH_CLASS_1_TARGET_RUNNER_DEFAULT;
    class_config.target_runner_override_in_direct_mode = DRV_RDD_IH_CLASS_1_TARGET_RUNNER_OVERRIDE_IN_DIRECT_MODE;
    class_config.target_runner_for_direct_mode = DRV_RDD_IH_CLASS_1_TARGET_RUNNER_FOR_DIRECT_MODE;
    class_config.load_balancing_enable = DRV_RDD_IH_CLASS_1_LOAD_BALANCING_ENABLE;
    class_config.preference_load_balancing_enable = DRV_RDD_IH_CLASS_1_PREFERENCE_LOAD_BALANCING_ENABLE;

    return fi_bl_drv_ih_configure_class(DRV_RDD_IH_CLASS_IPTV_INDEX, &class_config);
}

void _rdpa_iptv_ih_cfg_iptv_lookup_table(rdpa_iptv_lookup_method lookup_method)
{
    DRV_IH_LOOKUP_TABLE_60_BIT_KEY_CONFIG lookup_table_60_bit_key_config = {};

    lookup_table_60_bit_key_config.table_base_address_in_8_byte =
        MS_BYTE_TO_8_BYTE_RESOLUTION(DRV_RDD_IH_LOOKUP_TABLE_2_BASE_ADDRESS);
    lookup_table_60_bit_key_config.table_size = DRV_RDD_IH_LOOKUP_TABLE_2_SIZE;
    lookup_table_60_bit_key_config.maximal_search_depth = DRV_RDD_IH_LOOKUP_TABLE_2_SEARCH_DEPTH;
    lookup_table_60_bit_key_config.hash_type = DRV_RDD_IH_LOOKUP_TABLE_2_HASH_TYPE;
    lookup_table_60_bit_key_config.sa_search_enable = DRV_RDD_IH_LOOKUP_TABLE_2_SA_ENABLE;
    lookup_table_60_bit_key_config.aging_enable = DRV_RDD_IH_LOOKUP_TABLE_2_AGING_ENABLE;
    lookup_table_60_bit_key_config.cam_enable = DRV_RDD_IH_LOOKUP_TABLE_2_CAM_ENABLE;
    lookup_table_60_bit_key_config.cam_base_address_in_8_byte =
        MS_BYTE_TO_8_BYTE_RESOLUTION(DRV_RDD_IH_LOOKUP_TABLE_2_CAM_BASE_ADDRESS);
    lookup_table_60_bit_key_config.context_table_base_address_in_8_byte =
        MS_BYTE_TO_8_BYTE_RESOLUTION(DRV_RDD_IH_CONTEXT_TABLE_2_BASE_ADDRESS);
    lookup_table_60_bit_key_config.context_table_entry_size = DRV_RDD_IH_CONTEXT_TABLE_2_ENTRY_SIZE;
    lookup_table_60_bit_key_config.cam_context_base_address_in_8_byte =
        MS_BYTE_TO_8_BYTE_RESOLUTION(DRV_RDD_IH_CONTEXT_TABLE_2_CAM_BASE_ADDRESS);

    switch (lookup_method)
    {
    case iptv_lookup_method_mac:
        lookup_table_60_bit_key_config.part_0_start_offset_in_4_byte =
            DRV_RDD_IH_LOOKUP_TABLE_2_IPTV_DA_KEY_PART_0_START_OFFSET;
        lookup_table_60_bit_key_config.part_0_shift_offset_in_4_bit =
            DRV_RDD_IH_LOOKUP_TABLE_2_IPTV_DA_KEY_PART_0_SHIFT;
        lookup_table_60_bit_key_config.part_1_start_offset_in_4_byte =
            DRV_RDD_IH_LOOKUP_TABLE_2_IPTV_DA_KEY_PART_1_START_OFFSET;
        lookup_table_60_bit_key_config.part_1_shift_offset_in_4_bit =
            DRV_RDD_IH_LOOKUP_TABLE_2_IPTV_DA_KEY_PART_1_SHIFT;
        lookup_table_60_bit_key_config.key_extension = DRV_RDD_IH_LOOKUP_TABLE_2_IPTV_DA_KEY_KEY_EXTENSION;
        lookup_table_60_bit_key_config.part_0_mask_low = DRV_RDD_IH_LOOKUP_TABLE_2_IPTV_DA_KEY_PART_0_MASK_LOW;
        lookup_table_60_bit_key_config.part_0_mask_high = DRV_RDD_IH_LOOKUP_TABLE_2_IPTV_DA_KEY_PART_0_MASK_HIGH;
        lookup_table_60_bit_key_config.part_1_mask_low = DRV_RDD_IH_LOOKUP_TABLE_2_IPTV_DA_KEY_PART_1_MASK_LOW;
        lookup_table_60_bit_key_config.part_1_mask_high = DRV_RDD_IH_LOOKUP_TABLE_2_IPTV_DA_KEY_PART_1_MASK_HIGH;
        lookup_table_60_bit_key_config.global_mask_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_2_IPTV_DA_KEY_GLOBAL_MASK;
        break;
    case iptv_lookup_method_mac_vid:
        lookup_table_60_bit_key_config.part_0_start_offset_in_4_byte =
            DRV_RDD_IH_LOOKUP_TABLE_2_IPTV_DA_VID_KEY_PART_0_START_OFFSET;
        lookup_table_60_bit_key_config.part_0_shift_offset_in_4_bit =
            DRV_RDD_IH_LOOKUP_TABLE_2_IPTV_DA_VID_KEY_PART_0_SHIFT;
        lookup_table_60_bit_key_config.part_1_start_offset_in_4_byte =
            DRV_RDD_IH_LOOKUP_TABLE_2_IPTV_DA_VID_KEY_PART_1_START_OFFSET;
        lookup_table_60_bit_key_config.part_1_shift_offset_in_4_bit =
            DRV_RDD_IH_LOOKUP_TABLE_2_IPTV_DA_VID_KEY_PART_1_SHIFT;
        lookup_table_60_bit_key_config.key_extension = DRV_RDD_IH_LOOKUP_TABLE_2_IPTV_DA_VID_KEY_KEY_EXTENSION;
        lookup_table_60_bit_key_config.part_0_mask_low = DRV_RDD_IH_LOOKUP_TABLE_2_IPTV_DA_VID_KEY_PART_0_MASK_LOW;
        lookup_table_60_bit_key_config.part_0_mask_high = DRV_RDD_IH_LOOKUP_TABLE_2_IPTV_DA_VID_KEY_PART_0_MASK_HIGH;
        lookup_table_60_bit_key_config.part_1_mask_low = DRV_RDD_IH_LOOKUP_TABLE_2_IPTV_DA_VID_KEY_PART_1_MASK_LOW;
        lookup_table_60_bit_key_config.part_1_mask_high = DRV_RDD_IH_LOOKUP_TABLE_2_IPTV_DA_VID_KEY_PART_1_MASK_HIGH;
        lookup_table_60_bit_key_config.global_mask_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_2_IPTV_DA_VID_KEY_GLOBAL_MASK;
        break;

    case iptv_lookup_method_group_ip:
    case iptv_lookup_method_group_ip_src_ip:
        lookup_table_60_bit_key_config.part_0_start_offset_in_4_byte =
            DRV_RDD_IH_LOOKUP_TABLE_2_IPTV_L3_DIP_KEY_PART_0_START_OFFSET;
        lookup_table_60_bit_key_config.part_0_shift_offset_in_4_bit =
            DRV_RDD_IH_LOOKUP_TABLE_2_IPTV_L3_DIP_KEY_PART_0_SHIFT;
        lookup_table_60_bit_key_config.part_1_start_offset_in_4_byte =
            DRV_RDD_IH_LOOKUP_TABLE_2_IPTV_L3_DIP_KEY_PART_1_START_OFFSET;
        lookup_table_60_bit_key_config.part_1_shift_offset_in_4_bit =
            DRV_RDD_IH_LOOKUP_TABLE_2_IPTV_L3_DIP_KEY_PART_1_SHIFT;
        lookup_table_60_bit_key_config.key_extension = DRV_RDD_IH_LOOKUP_TABLE_2_IPTV_L3_DIP_KEY_KEY_EXTENSION;
        lookup_table_60_bit_key_config.part_0_mask_low = DRV_RDD_IH_LOOKUP_TABLE_2_IPTV_L3_DIP_KEY_PART_0_MASK_LOW;
        lookup_table_60_bit_key_config.part_0_mask_high = DRV_RDD_IH_LOOKUP_TABLE_2_IPTV_L3_DIP_KEY_PART_0_MASK_HIGH;
        lookup_table_60_bit_key_config.part_1_mask_low = DRV_RDD_IH_LOOKUP_TABLE_2_IPTV_L3_DIP_KEY_PART_1_MASK_LOW;
        lookup_table_60_bit_key_config.part_1_mask_high = DRV_RDD_IH_LOOKUP_TABLE_2_IPTV_L3_DIP_KEY_PART_1_MASK_HIGH;
        lookup_table_60_bit_key_config.global_mask_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_2_IPTV_L3_DIP_KEY_GLOBAL_MASK;
        break;
    case iptv_lookup_method_group_ip_src_ip_vid:
        lookup_table_60_bit_key_config.part_0_start_offset_in_4_byte =
            DRV_RDD_IH_LOOKUP_TABLE_2_IPTV_L3_DIP_SIP_VID_KEY_PART_0_START_OFFSET;
        lookup_table_60_bit_key_config.part_0_shift_offset_in_4_bit =
            DRV_RDD_IH_LOOKUP_TABLE_2_IPTV_L3_DIP_SIP_VID_KEY_PART_0_SHIFT;
        lookup_table_60_bit_key_config.part_1_start_offset_in_4_byte =
            DRV_RDD_IH_LOOKUP_TABLE_2_IPTV_L3_DIP_SIP_VID_KEY_PART_1_START_OFFSET;
        lookup_table_60_bit_key_config.part_1_shift_offset_in_4_bit =
            DRV_RDD_IH_LOOKUP_TABLE_2_IPTV_L3_DIP_SIP_VID_KEY_PART_1_SHIFT;
        lookup_table_60_bit_key_config.key_extension = DRV_RDD_IH_LOOKUP_TABLE_2_IPTV_L3_DIP_SIP_VID_KEY_KEY_EXTENSION;
        lookup_table_60_bit_key_config.part_0_mask_low =
            DRV_RDD_IH_LOOKUP_TABLE_2_IPTV_L3_DIP_SIP_VID_KEY_PART_0_MASK_LOW;
        lookup_table_60_bit_key_config.part_0_mask_high =
            DRV_RDD_IH_LOOKUP_TABLE_2_IPTV_L3_DIP_SIP_VID_KEY_PART_0_MASK_HIGH;
        lookup_table_60_bit_key_config.part_1_mask_low =
            DRV_RDD_IH_LOOKUP_TABLE_2_IPTV_L3_DIP_SIP_VID_KEY_PART_1_MASK_LOW;
        lookup_table_60_bit_key_config.part_1_mask_high =
            DRV_RDD_IH_LOOKUP_TABLE_2_IPTV_L3_DIP_SIP_VID_KEY_PART_1_MASK_HIGH;
        lookup_table_60_bit_key_config.global_mask_in_4_bit =
            DRV_RDD_IH_LOOKUP_TABLE_2_IPTV_L3_DIP_SIP_VID_KEY_GLOBAL_MASK;
        break;
    }

    fi_bl_drv_ih_configure_lut_60_bit_key(DRV_RDD_IH_LOOKUP_TABLE_IPTV_INDEX, &lookup_table_60_bit_key_config);
}

static int _rdpa_iptv_cfg_lookup_method_rdd_ih(iptv_drv_priv_t *iptv_cfg)
{
    int rc;
    DRV_IH_CLASS_CONFIG ih_class_cfg;

    rdd_iptv_lkp_method_cfg(iptv_cfg->lookup_method);

    /* Reconfigure IPTV classifier in IH */
    fi_bl_drv_ih_get_class_configuration(DRV_RDD_IH_CLASS_IPTV_INDEX, &ih_class_cfg);
    ih_class_cfg.class_search_2 = IPTV_IS_SRC_USED(iptv_cfg) ?
        DRV_RDD_IH_LOOKUP_TABLE_IPTV_SRC_IP_INDEX : DRV_IH_CLASS_SEARCH_DISABLED;
    rc = fi_bl_drv_ih_configure_class(DRV_RDD_IH_CLASS_IPTV_INDEX, &ih_class_cfg);
    if (rc)
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, iptv_object, "Failed to change IPTV class configuration in IH, "
            "rdd err %d", rc);
    }
    _rdpa_ih_cfg_iptv_lookup_classifier();

    /* Reconfigure IPTV lookup table */
    _rdpa_iptv_ih_cfg_iptv_lookup_table(iptv_cfg->lookup_method);
    return BDMF_ERR_OK;
}

/* IH IP filter id enumeration */
typedef enum
{
    iptv_ih_ip_filter_0, /**< IH IP filter id 0 */
    iptv_ih_ip_filter_1, /**< IH IP filter id 1 */
    iptv_ih_ip_filter_2, /**< IH IP filter id 2 */
    iptv_ih_ip_filter_3, /**< IH IP filter id 3 */
} rdpa_iptv_ip_filter_id;

static void _rdpa_iptv_ih_ip_filter_config(bdmf_boolean enable)
{
    uint32_t i;
    DRV_IH_CLASSIFIER_CONFIG ih_classifier_config;
    /* Ipv4 multicast range - 224.0.0.0 to 239.255.255.255, mask = 240.0.0.0 */
    uint32_t ipv4_mc_subnet = 0xe0000000, ipv4_mc_subnet_mask = 0xf0000000;
    /* Ipv6 multicast range - FF00:: to FF0F:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF */
    /*                        and FF30:: to FF3F:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF  */
    /*                        mask = FFF0:: */
    uint32_t ipv6_mc_subnet_1 = 0xff000000, ipv6_mc_subnet_2 = 0xff300000, ipv6_mc_subnet_mask = 0xfff00000;

    /* Configure ih ip filter */
    if (enable)
    {
        fi_bl_drv_ih_set_ip_filter(iptv_ih_ip_filter_0, ipv4_mc_subnet, ipv4_mc_subnet_mask,
            DRV_IH_IP_FILTER_SELECTION_DESTINATION_IP);
        fi_bl_drv_ih_set_ip_filter(iptv_ih_ip_filter_1, ipv6_mc_subnet_1, ipv6_mc_subnet_mask,
            DRV_IH_IP_FILTER_SELECTION_DESTINATION_IP);
        fi_bl_drv_ih_set_ip_filter(iptv_ih_ip_filter_2, ipv6_mc_subnet_2, ipv6_mc_subnet_mask,
            DRV_IH_IP_FILTER_SELECTION_DESTINATION_IP);
    }

    for (i = iptv_ih_ip_filter_0; i < iptv_ih_ip_filter_3; ++i)
        fi_bl_drv_ih_enable_ip_filter(i, enable);

    /* Configure all ih IP filters classifier */
    memset(&ih_classifier_config, 0, sizeof(DRV_IH_CLASSIFIER_CONFIG));

    ih_classifier_config.mask = RDPA_IPTV_FILTER_MASK_IP_FILTER_ANYHIT;
    ih_classifier_config.resulting_class = DRV_RDD_IH_CLASS_IPTV_INDEX;
    ih_classifier_config.ip_filter_any_hit = 1;
    /* Configure a classifier for broadcast traffic arriving on IPTV flow */
    if (enable)
        fi_bl_drv_ih_configure_classifier(RDPA_IH_CLASSIFIER_IPTV, &ih_classifier_config);
    else
        fi_bl_drv_ih_remove_classifier(RDPA_IH_CLASSIFIER_IPTV);
}

/* Enable IPTV prefix filter in IH. */
static void _rdpa_iptv_ih_mac_filter_config(bdmf_boolean enable)
{
    DRV_IH_CLASSIFIER_CONFIG ih_classifier_config;
    uint8_t ipv4_mac_da_prefix[] = {0x01, 0x00, 0x5E, 0x00, 0x00, 0x00};
    uint8_t ipv4_mac_da_prefix_mask[] = {0xFF, 0xFF, 0xFF, 0x80, 0x00, 0x00};
    uint8_t ipv6_mac_da_prefix[] = {0x33, 0x33, 0x00, 0x00, 0x00, 0x00};
    uint8_t ipv6_mac_da_prefix_mask[] = {0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00};

    /* We're using 2 DA filters with mask, 1 for IPv4 and 1 for IPv6 */
    fi_bl_drv_ih_set_da_filter_with_mask(RDPA_IH_DA_FILTER_IPTV_IPV4, ipv4_mac_da_prefix, ipv4_mac_da_prefix_mask);
    fi_bl_drv_ih_set_da_filter_with_mask(RDPA_IH_DA_FILTER_IPTV_IPV6, ipv6_mac_da_prefix, ipv6_mac_da_prefix_mask);

    /* Enable the IH DA filter - IPv4: 0x01005E */
    fi_bl_drv_ih_enable_da_filter(RDPA_IH_DA_FILTER_IPTV_IPV4, enable);

    /* Enable the IH DA filter - IPv6: 0x3333 */
    fi_bl_drv_ih_enable_da_filter(RDPA_IH_DA_FILTER_IPTV_IPV6, enable);

    /* Clear all attributes */
    memset(&ih_classifier_config, 0, sizeof(DRV_IH_CLASSIFIER_CONFIG));

    /* When IPTV prefix filter is enabled: We want to override the class to IPTV class  */
    ih_classifier_config.da_filter_any_hit = 1;
    /* da_filter_hit set to 1 and da_filter_number mask is[110] because we're after filters 0-1 */
    ih_classifier_config.mask = RDPA_IPTV_FILTER_MASK_DA;
    ih_classifier_config.resulting_class = DRV_RDD_IH_CLASS_IPTV_INDEX;
    ih_classifier_config.matched_da_filter = 0;

    /* Configure a classifier for IPTV */
    if (enable)
        fi_bl_drv_ih_configure_classifier(RDPA_IH_CLASSIFIER_IPTV, &ih_classifier_config);
    else
        fi_bl_drv_ih_remove_classifier(RDPA_IH_CLASSIFIER_IPTV);
}

int rdpa_iptv_post_init_ex(void)
{
    bdmf_error_t rc;

    rc = _rdpa_iptv_ih_class_config();
    rc = rc ? rc : _rdpa_iptv_group_src_ip_lookup_table_config();
    rc = rc ? rc : _rdpa_iptv_ds_ic_classification_lookup_table_config();
    return rc;
}

void rdpa_iptv_destroy_ex(void)
{
    return;
}

int rdpa_iptv_cfg_rdd_update_ex(iptv_drv_priv_t *iptv_cfg, iptv_drv_priv_t *new_iptv_cfg, bdmf_boolean post_init)
{
    bdmf_error_t rc;

    /* Change only if not equal or init time */
    if (new_iptv_cfg->mcast_prefix_filter != iptv_cfg->mcast_prefix_filter || post_init)
    {
        if (!post_init)
        {
            /* First roll-back previous configuration */
            if (iptv_cfg->mcast_prefix_filter == rdpa_mcast_filter_method_mac)
                _rdpa_iptv_ih_mac_filter_config(0);
            else if (iptv_cfg->mcast_prefix_filter == rdpa_mcast_filter_method_ip)
                _rdpa_iptv_ih_ip_filter_config(0);
        }

        switch (new_iptv_cfg->mcast_prefix_filter)
        {
        case rdpa_mcast_filter_method_mac:
            _rdpa_iptv_ih_mac_filter_config(1);
            break;
        case rdpa_mcast_filter_method_ip:
            _rdpa_iptv_ih_ip_filter_config(1);
            break;
        default:
            break;
        }
    }

    /* Change only if not equal or init time */
    if (iptv_cfg->lookup_method != new_iptv_cfg->lookup_method || post_init)
    {
        rc = _rdpa_iptv_cfg_lookup_method_rdd_ih(new_iptv_cfg);
        if (rc < 0)
            return rc;
    }
    return BDMF_ERR_OK;
}

int rdpa_iptv_rdd_entry_get_ex(uint32_t channel_index, rdd_iptv_entry_t *rdd_iptv_entry)
{
    return rdd_iptv_entry_get(channel_index, rdd_iptv_entry);
}

static void _rdpa_iptv_update_ddr_lookup(bdmf_boolean add_channel_to_ddr)
{
    iptv_drv_priv_t *iptv_cfg = (iptv_drv_priv_t *)bdmf_obj_data(iptv_object);
    DRV_IH_CLASS_CONFIG ih_class_cfg;

    if (add_channel_to_ddr)
        iptv_cfg->channels_in_ddr++;
    else
        iptv_cfg->channels_in_ddr--;
    if (iptv_cfg->channels_in_ddr > 1)
        return;

    /* This is either first or last multicast entry in DDR, need to disable/enable IPTV class predefined lookup
     * search (for entries stored in DDR, IH should perform non-drop search). */
    fi_bl_drv_ih_get_class_configuration(DRV_RDD_IH_CLASS_IPTV_INDEX, &ih_class_cfg);
    ih_class_cfg.drop_on_miss = iptv_cfg->channels_in_ddr ?
        DRV_IH_OPERATION_BASED_ON_CLASS_SEARCH_OPERATION_DISABLED :
        DRV_IH_OPERATION_BASED_ON_CLASS_SEARCH_BASED_ON_SEARCH1;
    fi_bl_drv_ih_configure_class(DRV_RDD_IH_CLASS_IPTV_INDEX, &ih_class_cfg);
}

int rdpa_iptv_rdd_entry_add_ex(rdd_iptv_entry_t *rdd_iptv_entry, uint32_t *channel_idx)
{
    bdmf_error_t rc;
    uint32_t is_in_cache = 0;

    rc = rdd_iptv_entry_add(rdd_iptv_entry, channel_idx, &is_in_cache);
    if (!rc && !is_in_cache)
        _rdpa_iptv_update_ddr_lookup(1);

    return rc;
}

int rdpa_iptv_rdd_entry_delete_ex(uint32_t channel_idx)
{
    bdmf_error_t rc;
    uint32_t is_in_cache = 0;

    rc = rdd_iptv_entry_delete(channel_idx, &is_in_cache);
    if (!rc && !is_in_cache)
        _rdpa_iptv_update_ddr_lookup(0);

    return rc;
}

int rdpa_iptv_rdd_entry_search_ex(rdpa_iptv_channel_key_t *key, uint32_t *index)
{
    bdmf_error_t rc;
    rdd_iptv_entry_t rdd_iptv_entry = {};

    rdpa_iptv_channel_key2rdd_iptv_entry_key(key, 0, 0, &rdd_iptv_entry);
    rc = rdd_iptv_entry_search(&rdd_iptv_entry, index);

    return rc;
}

int rdpa_iptv_ic_result_add_ex(mcast_result_entry_t *entry)
{
    /* XXX: Change the implementation of rdpa_ic_result_add get the vlan action per port from mcast result */
    return rdpa_ic_result_add(entry->mcast_result_idx, rdpa_dir_ds, &entry->mcast_result, 1, RDPA_IC_TYPE_FLOW);
}

void rdpa_iptv_ic_result_delete_ex(uint32_t mcast_result_idx, rdpa_traffic_dir dir)
{
    rdpa_ic_result_delete(mcast_result_idx, dir);
}

int rdpa_iptv_stat_read_ex(struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    bdmf_error_t rc = BDMF_ERR_OK;
#ifndef RDP_SIM
    uint32_t iptv_filter_counter = 0;
    rdd_various_counters_t various_counters = {};
    rdd_vport_pm_counters_t port_counters = {};
    uint16_t crc_counter = 0;
    rdpa_iptv_stat_t *stat = (rdpa_iptv_stat_t *)val;

    memset(stat, 0, sizeof(rdpa_iptv_stat_t));

    /* RDD bridge port counters */
#ifdef LEGACY_RDP
    rc = rdd_bridge_port_pm_counters_get(BL_LILAC_RDD_WAN_IPTV_BRIDGE_PORT, 0, &port_counters);
#else
    rc = rdd_iptv_rx_pm_counters_get(&port_counters);
#endif
    if (rc)
       BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "can't read RDD port counters, error = %d\n", rc);

    /* BBH IPTV counters */
    rc = fi_bl_drv_bbh_rx_get_iptv_filter_counter(rdpa_if_to_bbh_emac(rdpa_if_wan0), /* FIXME: MULTI-WAN XPON */
        &iptv_filter_counter);
    if (rc)
       BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "can't read BBH Rx counters, error = %d\n", rc);

    /* RDD iptv drop counter */
    /* TODO - we may need a shadowing here */
    rc = rdd_various_counters_get(rdpa_dir_ds, IPTV_L3_DROP_COUNTER_MASK, 0, &various_counters);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "can't read RDD drop counters, error = %d\n", rc);

    /* CRC error counter */
    rc = rdd_crc_err_counter_get(BL_LILAC_RDD_WAN_IPTV_BRIDGE_PORT, 0, &crc_counter);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "can't read RDD CRC counters, error = %d\n", rc);

    accumulative_iptv_filter_counter += iptv_filter_counter;
    stat->rx_valid_pkt = port_counters.rx_valid;
    stat->discard_pkt = port_counters.bridge_filtered_packets + accumulative_iptv_filter_counter +
        various_counters.iptv_layer3_drop;
    stat->rx_crc_error_pkt = crc_counter;
    stat->rx_valid_bytes = 0; /* not supported in RDP*/
#endif
    return rc;
}

/* "stat" attribute "write" callback */
int iptv_attr_stat_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, const void *val,
    uint32_t size)
{
    int rc = BDMF_ERR_OK;
#if defined LEGACY_RDP && !defined RDP_SIM
    rdd_various_counters_t various_counters = {};
    rdd_vport_pm_counters_t port_counters = {};
    uint16_t crc_counter = 0;
    rc = rdd_bridge_port_pm_counters_get(BL_LILAC_RDD_WAN_IPTV_BRIDGE_PORT, 1, &port_counters);
    if (rc)
       BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "can't clear PM port counters, error = %d\n", rc);
    rc = rdd_various_counters_get(rdpa_dir_ds, IPTV_L3_DROP_COUNTER_MASK, 1, &various_counters);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "can't clear IPTV drop counter, error = %d\n", rc);
    rc = rdd_crc_err_counter_get(BL_LILAC_RDD_WAN_IPTV_BRIDGE_PORT, 1, &crc_counter);
    if (rc)
       BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "can't CRC error counter, error = %d\n", rc);
#endif
    accumulative_iptv_filter_counter = 0;
    return rc;
}

bdmf_error_t rdpa_iptv_channel_rdd_pm_stat_get_ex(bdmf_index channel_index, rdpa_stat_t *pm_stat)
{
#if !defined(RDP_SIM)
    int rc;
    uint16_t packets;

    rc = rdd_iptv_counter_get((uint32_t)channel_index, &packets);
    if (rc)
    {
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Failed to read iptv counter, idx %d, rdd error %d\n",
            (uint32_t)channel_index, rc);
    }
    /* RDD doesn't supply the counter of received bytes, only packets. */
    pm_stat->packets = packets;
    pm_stat->bytes = 0;
    return BDMF_ERR_OK;
#else
    return BDMF_ERR_NOT_SUPPORTED;
#endif
}

#define LAN_VLAN_ACTION_FMT "_vlan_action={vlan_action/dir=ds,index=%d}"
bdmf_error_t rdpa_iptv_channel_mcast_result_val_to_str_ex(const void *val, char *sbuf,
    uint32_t _size, rdd_ic_context_t *rdd_classify_ctx, rdpa_ports ports)
{
    rdpa_ic_result_t *mcast_result = (rdpa_ic_result_t *)val;
    char tmp[128];
    int rc, size = (int)_size;

#ifndef G9991
    rc = snprintf(sbuf, size, "{qos_method=%s,forward_action=%s,tc=%d,per_port_vlan_actions={"
        "lan0" LAN_VLAN_ACTION_FMT ",lan1" LAN_VLAN_ACTION_FMT ",lan2" LAN_VLAN_ACTION_FMT
        ",lan3" LAN_VLAN_ACTION_FMT ",pci" LAN_VLAN_ACTION_FMT,
        bdmf_attr_get_enum_text_hlp(&rdpa_qos_method_enum_table, mcast_result->qos_method),
        bdmf_attr_get_enum_text_hlp(&rdpa_forward_action_enum_table, mcast_result->action),
        rdd_classify_ctx->priority,
        rdd_classify_ctx->ds_eth0_vlan_cmd,
        rdd_classify_ctx->ds_eth1_vlan_cmd,
        rdd_classify_ctx->ds_eth2_vlan_cmd,
        rdd_classify_ctx->ds_eth3_vlan_cmd,
        rdd_classify_ctx->ds_pci_vlan_cmd);
#else
    rc = snprintf(sbuf, size, "{qos_method=%s,forward_action=%s,tc=%d,per_port_vlan_actions={"
        "lan0" LAN_VLAN_ACTION_FMT ",lan1" LAN_VLAN_ACTION_FMT ",lan2" LAN_VLAN_ACTION_FMT
        ",lan3" LAN_VLAN_ACTION_FMT,
        bdmf_attr_get_enum_text_hlp(&rdpa_qos_method_enum_table, mcast_result->qos_method),
        bdmf_attr_get_enum_text_hlp(&rdpa_forward_action_enum_table, mcast_result->action),
        rdd_classify_ctx->priority,
        rdd_classify_ctx->ds_eth0_vlan_cmd,
        rdd_classify_ctx->ds_eth1_vlan_cmd,
        rdd_classify_ctx->ds_eth2_vlan_cmd,
        rdd_classify_ctx->ds_eth3_vlan_cmd);
#endif
    size -= rc;
    if (size < 0)
        return BDMF_ERR_INTERNAL;

    if (rdd_classify_ctx->opbit_remark)
        rc = sprintf(tmp, ",opbit_remark=yes,opbit_val=%d", (int)rdd_classify_ctx->opbit_val);
    else
        rc = sprintf(tmp, ",opbit_remark=no");
    strncat(sbuf, tmp, size);
    size -= rc;
    if (size < 0)
        return BDMF_ERR_INTERNAL;

    if (rdd_classify_ctx->ipbit_remark)
        rc = sprintf(tmp, ",ipbit_remark=yes,ipbit_val=%d", (int)rdd_classify_ctx->ipbit_val);
    else
        rc = sprintf(tmp, ",ipbit_remark=no");
    strncat(sbuf, tmp, size);
    size -= rc;
    if (size < 0)
        return BDMF_ERR_INTERNAL;

    if (rdd_classify_ctx->dscp_remark)
    {
        rc = sprintf(tmp, ",dscp_remark=yes,dscp_val=%d,ecn_val=%d",
            rdd_classify_ctx->dscp_val, rdd_classify_ctx->ecn_val);
    }
    else
        rc = sprintf(tmp, ",dscp_remark=no");
    strncat(sbuf, tmp, size);
    size -= rc;
    /* add info about service queue */
    if (rdd_classify_ctx->service_queue_mode)
        rc = sprintf(tmp, ",service_queue_id=%d", (int)rdd_classify_ctx->service_queue);
    else
        rc = sprintf(tmp, ",service_queue_id=disable");
    strncat(sbuf, tmp, size);
    size -= rc;

    if (size < 0)
        return BDMF_ERR_INTERNAL;

    return BDMF_ERR_OK;
}

bdmf_error_t rdpa_iptv_lkp_miss_action_write_ex(rdpa_forward_action new_lookup_miss_action)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

