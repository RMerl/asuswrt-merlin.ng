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

#include "rdd_ag_ds_tm.h"

int rdd_ag_ds_tm_bb_destination_table_set(uint16_t bits)
{
    RDD_BYTES_2_BITS_WRITE_G(bits, RDD_DS_TM_BB_DESTINATION_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_bb_destination_table_set_core(uint16_t bits, int core_id)
{
    RDD_BYTES_2_BITS_WRITE_CORE(bits, RDD_DS_TM_BB_DESTINATION_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_bb_destination_table_get(uint16_t *bits)
{
    RDD_BYTES_2_BITS_READ_G(*bits, RDD_DS_TM_BB_DESTINATION_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_bb_destination_table_get_core(uint16_t *bits, int core_id)
{
    RDD_BYTES_2_BITS_READ_CORE(*bits, RDD_DS_TM_BB_DESTINATION_TABLE_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_first_queue_mapping_set(uint16_t bits)
{
    RDD_BYTES_2_BITS_WRITE_G(bits, RDD_DS_TM_FIRST_QUEUE_MAPPING_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_first_queue_mapping_set_core(uint16_t bits, int core_id)
{
    RDD_BYTES_2_BITS_WRITE_CORE(bits, RDD_DS_TM_FIRST_QUEUE_MAPPING_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_first_queue_mapping_get(uint16_t *bits)
{
    RDD_BYTES_2_BITS_READ_G(*bits, RDD_DS_TM_FIRST_QUEUE_MAPPING_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_first_queue_mapping_get_core(uint16_t *bits, int core_id)
{
    RDD_BYTES_2_BITS_READ_CORE(*bits, RDD_DS_TM_FIRST_QUEUE_MAPPING_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_vport_to_rl_overhead_table_set(uint32_t _entry, uint8_t rl_overhead)
{
    if(_entry >= RDD_VPORT_TO_RL_OVERHEAD_TABLE_SIZE)
          return BDMF_ERR_PARM;

    RDD_VPORT_TO_RL_OVERHEAD_ENTRY_RL_OVERHEAD_WRITE_G(rl_overhead, RDD_VPORT_TO_RL_OVERHEAD_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_vport_to_rl_overhead_table_set_core(uint32_t _entry, uint8_t rl_overhead, int core_id)
{
    if(_entry >= RDD_VPORT_TO_RL_OVERHEAD_TABLE_SIZE)
          return BDMF_ERR_PARM;

    RDD_VPORT_TO_RL_OVERHEAD_ENTRY_RL_OVERHEAD_WRITE_CORE(rl_overhead, RDD_VPORT_TO_RL_OVERHEAD_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_vport_to_rl_overhead_table_get(uint32_t _entry, uint8_t *rl_overhead)
{
    if(_entry >= RDD_VPORT_TO_RL_OVERHEAD_TABLE_SIZE)
          return BDMF_ERR_PARM;

    RDD_VPORT_TO_RL_OVERHEAD_ENTRY_RL_OVERHEAD_READ_G(*rl_overhead, RDD_VPORT_TO_RL_OVERHEAD_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_vport_to_rl_overhead_table_get_core(uint32_t _entry, uint8_t *rl_overhead, int core_id)
{
    if(_entry >= RDD_VPORT_TO_RL_OVERHEAD_TABLE_SIZE)
          return BDMF_ERR_PARM;

    RDD_VPORT_TO_RL_OVERHEAD_ENTRY_RL_OVERHEAD_READ_CORE(*rl_overhead, RDD_VPORT_TO_RL_OVERHEAD_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_vport_to_rl_overhead_table_rl_overhead_set(uint32_t _entry, uint8_t rl_overhead)
{
    if(_entry >= RDD_VPORT_TO_RL_OVERHEAD_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_TO_RL_OVERHEAD_ENTRY_RL_OVERHEAD_WRITE_G(rl_overhead, RDD_VPORT_TO_RL_OVERHEAD_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_vport_to_rl_overhead_table_rl_overhead_set_core(uint32_t _entry, uint8_t rl_overhead, int core_id)
{
    if(_entry >= RDD_VPORT_TO_RL_OVERHEAD_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_TO_RL_OVERHEAD_ENTRY_RL_OVERHEAD_WRITE_CORE(rl_overhead, RDD_VPORT_TO_RL_OVERHEAD_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_vport_to_rl_overhead_table_rl_overhead_get(uint32_t _entry, uint8_t *rl_overhead)
{
    if(_entry >= RDD_VPORT_TO_RL_OVERHEAD_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_TO_RL_OVERHEAD_ENTRY_RL_OVERHEAD_READ_G(*rl_overhead, RDD_VPORT_TO_RL_OVERHEAD_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_ds_tm_vport_to_rl_overhead_table_rl_overhead_get_core(uint32_t _entry, uint8_t *rl_overhead, int core_id)
{
    if(_entry >= RDD_VPORT_TO_RL_OVERHEAD_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_VPORT_TO_RL_OVERHEAD_ENTRY_RL_OVERHEAD_READ_CORE(*rl_overhead, RDD_VPORT_TO_RL_OVERHEAD_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

