/*
   Copyright (c) 2025 Broadcom
   All Rights Reserved

    <:label-BRCM:2025:DUAL/GPL:standard

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



/* This is an automated file. Do not edit its contents. */


#include "rdd.h"

#include "rdd_ag_us_tm.h"

int rdd_ag_us_tm_bbh_tx_us_wan_0_fifo_bytes_threshold_set(uint16_t bits)
{
    RDD_BYTES_2_BITS_WRITE_G(bits, RDD_BBH_TX_US_WAN_0_FIFO_BYTES_THRESHOLD_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_us_tm_bbh_tx_us_wan_0_fifo_bytes_threshold_set_core(uint16_t bits, int core_id)
{
    RDD_BYTES_2_BITS_WRITE_CORE(bits, RDD_BBH_TX_US_WAN_0_FIFO_BYTES_THRESHOLD_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_us_tm_bbh_tx_us_wan_0_fifo_bytes_threshold_get(uint16_t *bits)
{
    RDD_BYTES_2_BITS_READ_G(*bits, RDD_BBH_TX_US_WAN_0_FIFO_BYTES_THRESHOLD_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_us_tm_bbh_tx_us_wan_0_fifo_bytes_threshold_get_core(uint16_t *bits, int core_id)
{
    RDD_BYTES_2_BITS_READ_CORE(*bits, RDD_BBH_TX_US_WAN_0_FIFO_BYTES_THRESHOLD_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_us_tm_first_queue_mapping_set(uint16_t bits)
{
    RDD_BYTES_2_BITS_WRITE_G(bits, RDD_US_TM_FIRST_QUEUE_MAPPING_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_us_tm_first_queue_mapping_set_core(uint16_t bits, int core_id)
{
    RDD_BYTES_2_BITS_WRITE_CORE(bits, RDD_US_TM_FIRST_QUEUE_MAPPING_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_us_tm_first_queue_mapping_get(uint16_t *bits)
{
    RDD_BYTES_2_BITS_READ_G(*bits, RDD_US_TM_FIRST_QUEUE_MAPPING_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_us_tm_first_queue_mapping_get_core(uint16_t *bits, int core_id)
{
    RDD_BYTES_2_BITS_READ_CORE(*bits, RDD_US_TM_FIRST_QUEUE_MAPPING_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_us_tm_tx_octets_counters_table_packets_set(uint32_t _entry, uint32_t packets)
{
    if(_entry >= RDD_US_TM_TX_OCTETS_COUNTERS_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_PACKETS_AND_BYTES_PACKETS_WRITE_G(packets, RDD_US_TM_TX_OCTETS_COUNTERS_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_us_tm_tx_octets_counters_table_packets_set_core(uint32_t _entry, uint32_t packets, int core_id)
{
    if(_entry >= RDD_US_TM_TX_OCTETS_COUNTERS_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_PACKETS_AND_BYTES_PACKETS_WRITE_CORE(packets, RDD_US_TM_TX_OCTETS_COUNTERS_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_us_tm_tx_octets_counters_table_packets_get(uint32_t _entry, uint32_t *packets)
{
    if(_entry >= RDD_US_TM_TX_OCTETS_COUNTERS_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_PACKETS_AND_BYTES_PACKETS_READ_G(*packets, RDD_US_TM_TX_OCTETS_COUNTERS_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_us_tm_tx_octets_counters_table_packets_get_core(uint32_t _entry, uint32_t *packets, int core_id)
{
    if(_entry >= RDD_US_TM_TX_OCTETS_COUNTERS_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_PACKETS_AND_BYTES_PACKETS_READ_CORE(*packets, RDD_US_TM_TX_OCTETS_COUNTERS_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_us_tm_tx_octets_counters_table_bytes_set(uint32_t _entry, uint32_t bytes)
{
    if(_entry >= RDD_US_TM_TX_OCTETS_COUNTERS_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_PACKETS_AND_BYTES_BYTES_WRITE_G(bytes, RDD_US_TM_TX_OCTETS_COUNTERS_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_us_tm_tx_octets_counters_table_bytes_set_core(uint32_t _entry, uint32_t bytes, int core_id)
{
    if(_entry >= RDD_US_TM_TX_OCTETS_COUNTERS_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_PACKETS_AND_BYTES_BYTES_WRITE_CORE(bytes, RDD_US_TM_TX_OCTETS_COUNTERS_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_us_tm_tx_octets_counters_table_bytes_get(uint32_t _entry, uint32_t *bytes)
{
    if(_entry >= RDD_US_TM_TX_OCTETS_COUNTERS_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_PACKETS_AND_BYTES_BYTES_READ_G(*bytes, RDD_US_TM_TX_OCTETS_COUNTERS_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_us_tm_tx_octets_counters_table_bytes_get_core(uint32_t _entry, uint32_t *bytes, int core_id)
{
    if(_entry >= RDD_US_TM_TX_OCTETS_COUNTERS_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_PACKETS_AND_BYTES_BYTES_READ_CORE(*bytes, RDD_US_TM_TX_OCTETS_COUNTERS_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_us_tm_vport_to_rl_overhead_table_set(uint8_t rl_overhead)
{
    RDD_VPORT_TO_RL_OVERHEAD_ENTRY_RL_OVERHEAD_WRITE_G(rl_overhead, RDD_PON_TM_VPORT_TO_RL_OVERHEAD_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_us_tm_vport_to_rl_overhead_table_set_core(uint8_t rl_overhead, int core_id)
{
    RDD_VPORT_TO_RL_OVERHEAD_ENTRY_RL_OVERHEAD_WRITE_CORE(rl_overhead, RDD_PON_TM_VPORT_TO_RL_OVERHEAD_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_us_tm_vport_to_rl_overhead_table_get(uint8_t *rl_overhead)
{
    RDD_VPORT_TO_RL_OVERHEAD_ENTRY_RL_OVERHEAD_READ_G(*rl_overhead, RDD_PON_TM_VPORT_TO_RL_OVERHEAD_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_us_tm_vport_to_rl_overhead_table_get_core(uint8_t *rl_overhead, int core_id)
{
    RDD_VPORT_TO_RL_OVERHEAD_ENTRY_RL_OVERHEAD_READ_CORE(*rl_overhead, RDD_PON_TM_VPORT_TO_RL_OVERHEAD_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_us_tm_vport_to_rl_overhead_table_rl_overhead_set(uint8_t rl_overhead)
{
    RDD_VPORT_TO_RL_OVERHEAD_ENTRY_RL_OVERHEAD_WRITE_G(rl_overhead, RDD_PON_TM_VPORT_TO_RL_OVERHEAD_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_us_tm_vport_to_rl_overhead_table_rl_overhead_set_core(uint8_t rl_overhead, int core_id)
{
    RDD_VPORT_TO_RL_OVERHEAD_ENTRY_RL_OVERHEAD_WRITE_CORE(rl_overhead, RDD_PON_TM_VPORT_TO_RL_OVERHEAD_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_us_tm_vport_to_rl_overhead_table_rl_overhead_get(uint8_t *rl_overhead)
{
    RDD_VPORT_TO_RL_OVERHEAD_ENTRY_RL_OVERHEAD_READ_G(*rl_overhead, RDD_PON_TM_VPORT_TO_RL_OVERHEAD_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_us_tm_vport_to_rl_overhead_table_rl_overhead_get_core(uint8_t *rl_overhead, int core_id)
{
    RDD_VPORT_TO_RL_OVERHEAD_ENTRY_RL_OVERHEAD_READ_CORE(*rl_overhead, RDD_PON_TM_VPORT_TO_RL_OVERHEAD_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_us_tm_wan_0_bbh_tx_fifo_size_set(uint8_t bits)
{
    RDD_BYTE_1_BITS_WRITE_G(bits, RDD_WAN_0_BBH_TX_FIFO_SIZE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_us_tm_wan_0_bbh_tx_fifo_size_set_core(uint8_t bits, int core_id)
{
    RDD_BYTE_1_BITS_WRITE_CORE(bits, RDD_WAN_0_BBH_TX_FIFO_SIZE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_us_tm_wan_0_bbh_tx_fifo_size_get(uint8_t *bits)
{
    RDD_BYTE_1_BITS_READ_G(*bits, RDD_WAN_0_BBH_TX_FIFO_SIZE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_us_tm_wan_0_bbh_tx_fifo_size_get_core(uint8_t *bits, int core_id)
{
    RDD_BYTE_1_BITS_READ_CORE(*bits, RDD_WAN_0_BBH_TX_FIFO_SIZE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_us_tm_wan_0_bb_destination_table_set(uint16_t bits)
{
    RDD_BYTES_2_BITS_WRITE_G(bits, RDD_US_TM_WAN_0_BB_DESTINATION_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_us_tm_wan_0_bb_destination_table_set_core(uint16_t bits, int core_id)
{
    RDD_BYTES_2_BITS_WRITE_CORE(bits, RDD_US_TM_WAN_0_BB_DESTINATION_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_us_tm_wan_0_bb_destination_table_get(uint16_t *bits)
{
    RDD_BYTES_2_BITS_READ_G(*bits, RDD_US_TM_WAN_0_BB_DESTINATION_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_us_tm_wan_0_bb_destination_table_get_core(uint16_t *bits, int core_id)
{
    RDD_BYTES_2_BITS_READ_CORE(*bits, RDD_US_TM_WAN_0_BB_DESTINATION_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

