/*
   <:copyright-BRCM:2014-2016:DUAL/GPL:standard
   
      Copyright (c) 2014-2016 Broadcom 
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
#include "rdpa_cpu_basic.h"
#include "rdd_l4_filters.h"

static inline int _rdd_l4_filter_set(rdd_l4_filter_t index, rdd_action action,
    uint8_t reason, rdpa_traffic_dir dir)
{
    RDD_DS_L4_FILTERS_CONTEXT_TABLE_DTS *ds_context_table_ptr;
    RDD_US_L4_FILTERS_CONTEXT_TABLE_DTS *us_context_table_ptr;
    RDD_FILTERS_CONTEXT_ENTRY_DTS *context_entry_ptr;

    if (action == ACTION_FORWARD && index != RDD_L4_FILTER_GRE && index != RDD_L4_FILTER_L3_IPV4 &&
        index != RDD_L4_FILTER_L3_IPV6)
    {
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "CAM insertion failed\n");
    }

    if (dir == rdpa_dir_ds)
    {
        ds_context_table_ptr = RDD_DS_L4_FILTERS_CONTEXT_TABLE_PTR();
        context_entry_ptr = &(ds_context_table_ptr->entry[index]);
    }
    else
    {
        us_context_table_ptr = RDD_US_L4_FILTERS_CONTEXT_TABLE_PTR();
        context_entry_ptr = &(us_context_table_ptr->entry[index]);
    }

    RDD_FILTERS_CONTEXT_ENTRY_ACTION_WRITE(action, context_entry_ptr);
    RDD_FILTERS_CONTEXT_ENTRY_REASON_WRITE(reason, context_entry_ptr);

    return 0;
}

#define _RDD_L4_FILTERS_LOOKUP_ENTRY_WRITE(table_ptr, index, field_name, val, mask) \
    do { \
        RDD_L4_FILTERS_LOOKUP_ENTRY_DTS *l4_filter_entry_ptr = &(table_ptr->entry[index]); \
        RDD_L4_FILTERS_LOOKUP_ENTRY_##field_name##_WRITE(val, l4_filter_entry_ptr); \
        RDD_L4_FILTERS_LOOKUP_ENTRY_##field_name##_MASK_WRITE(mask, l4_filter_entry_ptr); \
    } while (0)

void rdd_l4_filters_init(rdpa_traffic_dir dir)
{
    RDD_DS_L4_FILTERS_LOOKUP_TABLE_DTS *lookup_table_ptr;
    rdd_l4_filter_t i;

    if (dir == rdpa_dir_ds)
        lookup_table_ptr = RDD_DS_L4_FILTERS_LOOKUP_TABLE_PTR();
    else
        lookup_table_ptr = (RDD_DS_L4_FILTERS_LOOKUP_TABLE_DTS *)RDD_US_L4_FILTERS_LOOKUP_TABLE_PTR();

    _RDD_L4_FILTERS_LOOKUP_ENTRY_WRITE(lookup_table_ptr, RDD_L4_FILTER_ERROR,
         ERROR, 0x1, 0x1);
    _RDD_L4_FILTERS_LOOKUP_ENTRY_WRITE(lookup_table_ptr, RDD_L4_FILTER_EXCEPTION,
         EXCEPTION, 0x1, 0x1);
    _RDD_L4_FILTERS_LOOKUP_ENTRY_WRITE(lookup_table_ptr, RDD_L4_FILTER_IP_FIRST_FRAGMENT,
         IP_FIRST_FRAGMENT, 0x1, 0x1);
    _RDD_L4_FILTERS_LOOKUP_ENTRY_WRITE(lookup_table_ptr, RDD_L4_FILTER_IP_FRAGMENT,
         IP_FRAGMENT, 0x1, 0x1);
    _RDD_L4_FILTERS_LOOKUP_ENTRY_WRITE(lookup_table_ptr, RDD_L4_FILTER_ICMP,
         L4_PROTOCOL, PARSER_L4_PROTOCOL_ICMP, 0xF);
    _RDD_L4_FILTERS_LOOKUP_ENTRY_WRITE(lookup_table_ptr, RDD_L4_FILTER_ESP,
         L4_PROTOCOL, PARSER_L4_PROTOCOL_ESP, 0xF);
    _RDD_L4_FILTERS_LOOKUP_ENTRY_WRITE(lookup_table_ptr, RDD_L4_FILTER_GRE,
         L4_PROTOCOL, PARSER_L4_PROTOCOL_GRE, 0xF);
    _RDD_L4_FILTERS_LOOKUP_ENTRY_WRITE(lookup_table_ptr, RDD_L4_FILTER_L3_IPV4,
         L3_PROTOCOL, PARSER_L3_PROTOCOL_IPV4, 0x3);
    _RDD_L4_FILTERS_LOOKUP_ENTRY_WRITE(lookup_table_ptr, RDD_L4_FILTER_L3_IPV4,
         L4_PROTOCOL, 0xC, 0x0);
    _RDD_L4_FILTERS_LOOKUP_ENTRY_WRITE(lookup_table_ptr, RDD_L4_FILTER_L3_IPV6,
         L3_PROTOCOL, PARSER_L3_PROTOCOL_IPV6, 0x3);
    _RDD_L4_FILTERS_LOOKUP_ENTRY_WRITE(lookup_table_ptr, RDD_L4_FILTER_L3_IPV6,
         L4_PROTOCOL, 0xC, 0x0);
    _RDD_L4_FILTERS_LOOKUP_ENTRY_WRITE(lookup_table_ptr, RDD_L4_FILTER_AH,
         L4_PROTOCOL, PARSER_L4_PROTOCOL_AH, 0xF);
    _RDD_L4_FILTERS_LOOKUP_ENTRY_WRITE(lookup_table_ptr, RDD_L4_FILTER_IPV6,
         L4_PROTOCOL, PARSER_L4_PROTOCOL_IPV6, 0xF);
    _RDD_L4_FILTERS_LOOKUP_ENTRY_WRITE(lookup_table_ptr, RDD_L4_FILTER_UDEF_0,
         L4_PROTOCOL, PARSER_L4_PROTOCOL_USER_DEFINED_0, 0xF);
    _RDD_L4_FILTERS_LOOKUP_ENTRY_WRITE(lookup_table_ptr, RDD_L4_FILTER_UDEF_1,
         L4_PROTOCOL, PARSER_L4_PROTOCOL_USER_DEFINED_1, 0xF);
    _RDD_L4_FILTERS_LOOKUP_ENTRY_WRITE(lookup_table_ptr, RDD_L4_FILTER_UDEF_2,
         L4_PROTOCOL, PARSER_L4_PROTOCOL_USER_DEFINED_2, 0xF);
    _RDD_L4_FILTERS_LOOKUP_ENTRY_WRITE(lookup_table_ptr, RDD_L4_FILTER_UDEF_3,
         L4_PROTOCOL, PARSER_L4_PROTOCOL_USER_DEFINED_3, 0xF);
    _RDD_L4_FILTERS_LOOKUP_ENTRY_WRITE(lookup_table_ptr, RDD_L4_FILTER_UNKNOWN,
             L4_PROTOCOL, PARSER_L4_PROTOCOL_OTHER, 0x0);

    for (i = 0; i <= RDD_L4_FILTER_UNKNOWN; i++)
    {
        _rdd_l4_filter_set(i, dir == rdpa_dir_ds ? ACTION_DROP : ACTION_TRAP,
            rdpa_cpu_rx_reason_non_tcp_udp, dir);
    }

    if (dir == rdpa_dir_us)
    {
        _rdd_l4_filter_set(RDD_L4_FILTER_L3_IPV4, ACTION_DROP,
            rdpa_cpu_rx_reason_non_tcp_udp, rdpa_dir_us);
        _rdd_l4_filter_set(RDD_L4_FILTER_L3_IPV6, ACTION_DROP,
            rdpa_cpu_rx_reason_non_tcp_udp, rdpa_dir_us);
    }
}

int rdd_l4_filter_set(rdd_l4_filter_t index, rdd_action action, uint8_t reason,
    rdpa_traffic_dir dir)
{
    return _rdd_l4_filter_set(index, action, reason, dir);
}

int rdd_hdr_err_filter_cfg(rdd_action action, uint8_t reason, rdpa_traffic_dir dir)
{
    if (action == ACTION_FORWARD)
        return BDMF_ERR_PARM;

    _rdd_l4_filter_set(RDD_L4_FILTER_ERROR, action, reason, dir);
    _rdd_l4_filter_set(RDD_L4_FILTER_EXCEPTION, action, reason, dir);
    return 0;
}

int rdd_ip_frag_filter_cfg(rdd_action action, uint8_t reason, rdpa_traffic_dir dir)
{
    if (action == ACTION_FORWARD)
        return BDMF_ERR_PARM;

    _rdd_l4_filter_set(RDD_L4_FILTER_IP_FIRST_FRAGMENT, action, reason, dir);
    _rdd_l4_filter_set(RDD_L4_FILTER_IP_FRAGMENT, action, reason, dir);

    return 0;
}
