/*
   Copyright (c) 2015 Broadcom
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard

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


#include "rdd_ag_tm.h"

int rdd_ag_tm_hw_flush_hw_flush_en_set(bdmf_boolean hw_flush_en)
{
    RDD_HW_FLUSH_ENTRY_HW_FLUSH_EN_WRITE_G(hw_flush_en, RDD_TM_HW_FLUSH_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_tm_hw_flush_hw_flush_en_get(bdmf_boolean *hw_flush_en)
{
    RDD_HW_FLUSH_ENTRY_HW_FLUSH_EN_READ_G(*hw_flush_en, RDD_TM_HW_FLUSH_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_tm_hw_flush_us_set(bdmf_boolean us)
{
    RDD_HW_FLUSH_ENTRY_US_WRITE_G(us, RDD_TM_HW_FLUSH_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_tm_hw_flush_us_get(bdmf_boolean *us)
{
    RDD_HW_FLUSH_ENTRY_US_READ_G(*us, RDD_TM_HW_FLUSH_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_tm_hw_flush_flush_aggr_set(uint8_t flush_aggr)
{
    RDD_HW_FLUSH_ENTRY_FLUSH_AGGR_WRITE_G(flush_aggr, RDD_TM_HW_FLUSH_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_tm_hw_flush_flush_aggr_get(uint8_t *flush_aggr)
{
    RDD_HW_FLUSH_ENTRY_FLUSH_AGGR_READ_G(*flush_aggr, RDD_TM_HW_FLUSH_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_tm_complex_scheduler_rl_cfg_get(uint32_t _entry, rdd_complex_scheduler_rl_cfg_t *complex_scheduler_rl_cfg)
{
    if(!complex_scheduler_rl_cfg || _entry >= RDD_COMPLEX_SCHEDULER_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_IS_POSITIVE_BUDGET_READ_G(complex_scheduler_rl_cfg->is_positive_budget, RDD_COMPLEX_SCHEDULER_TABLE_ADDRESS_ARR, _entry);
    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_RATE_LIMITER_INDEX_READ_G(complex_scheduler_rl_cfg->rate_limiter_index, RDD_COMPLEX_SCHEDULER_TABLE_ADDRESS_ARR, _entry);
    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_RATE_LIMIT_ENABLE_READ_G(complex_scheduler_rl_cfg->rate_limit_enable, RDD_COMPLEX_SCHEDULER_TABLE_ADDRESS_ARR, _entry);
    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DEFICIT_COUNTER_READ_G(complex_scheduler_rl_cfg->deficit_counter, RDD_COMPLEX_SCHEDULER_TABLE_ADDRESS_ARR, _entry);
    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_QUANTUM_NUMBER_READ_G(complex_scheduler_rl_cfg->quantum_number, RDD_COMPLEX_SCHEDULER_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_tm_complex_scheduler_rl_cfg_set(uint32_t _entry, rdd_complex_scheduler_rl_cfg_t *complex_scheduler_rl_cfg)
{
    if(!complex_scheduler_rl_cfg || _entry >= RDD_COMPLEX_SCHEDULER_TABLE_SIZE || complex_scheduler_rl_cfg->rate_limiter_index >= 128)
          return BDMF_ERR_PARM;

    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_IS_POSITIVE_BUDGET_WRITE_G(complex_scheduler_rl_cfg->is_positive_budget, RDD_COMPLEX_SCHEDULER_TABLE_ADDRESS_ARR, _entry);
    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_RATE_LIMITER_INDEX_WRITE_G(complex_scheduler_rl_cfg->rate_limiter_index, RDD_COMPLEX_SCHEDULER_TABLE_ADDRESS_ARR, _entry);
    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_RATE_LIMIT_ENABLE_WRITE_G(complex_scheduler_rl_cfg->rate_limit_enable, RDD_COMPLEX_SCHEDULER_TABLE_ADDRESS_ARR, _entry);
    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DEFICIT_COUNTER_WRITE_G(complex_scheduler_rl_cfg->deficit_counter, RDD_COMPLEX_SCHEDULER_TABLE_ADDRESS_ARR, _entry);
    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_QUANTUM_NUMBER_WRITE_G(complex_scheduler_rl_cfg->quantum_number, RDD_COMPLEX_SCHEDULER_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_tm_complex_scheduler_table_dwrr_offset_pir_set(uint32_t _entry, uint8_t dwrr_offset_pir)
{
    if(_entry >= RDD_COMPLEX_SCHEDULER_TABLE_SIZE || dwrr_offset_pir >= 8)
          return BDMF_ERR_PARM;

    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DWRR_OFFSET_PIR_WRITE_G(dwrr_offset_pir, RDD_COMPLEX_SCHEDULER_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_tm_complex_scheduler_table_dwrr_offset_pir_get(uint32_t _entry, uint8_t *dwrr_offset_pir)
{
    if(_entry >= RDD_COMPLEX_SCHEDULER_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DWRR_OFFSET_PIR_READ_G(*dwrr_offset_pir, RDD_COMPLEX_SCHEDULER_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_tm_complex_scheduler_table_dwrr_offset_sir_set(uint32_t _entry, uint8_t dwrr_offset_sir)
{
    if(_entry >= RDD_COMPLEX_SCHEDULER_TABLE_SIZE || dwrr_offset_sir >= 8)
          return BDMF_ERR_PARM;

    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DWRR_OFFSET_SIR_WRITE_G(dwrr_offset_sir, RDD_COMPLEX_SCHEDULER_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_tm_complex_scheduler_table_dwrr_offset_sir_get(uint32_t _entry, uint8_t *dwrr_offset_sir)
{
    if(_entry >= RDD_COMPLEX_SCHEDULER_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DWRR_OFFSET_SIR_READ_G(*dwrr_offset_sir, RDD_COMPLEX_SCHEDULER_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_tm_complex_scheduler_table_slot_budget_bit_vector_0_set(uint32_t _entry, uint32_t slot_budget_bit_vector_0)
{
    if(_entry >= RDD_COMPLEX_SCHEDULER_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_SLOT_BUDGET_BIT_VECTOR_0_WRITE_G(slot_budget_bit_vector_0, RDD_COMPLEX_SCHEDULER_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_tm_complex_scheduler_table_slot_budget_bit_vector_0_get(uint32_t _entry, uint32_t *slot_budget_bit_vector_0)
{
    if(_entry >= RDD_COMPLEX_SCHEDULER_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_SLOT_BUDGET_BIT_VECTOR_0_READ_G(*slot_budget_bit_vector_0, RDD_COMPLEX_SCHEDULER_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_tm_complex_scheduler_table_quantum_number_set(uint32_t _entry, uint8_t quantum_number)
{
    if(_entry >= RDD_COMPLEX_SCHEDULER_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_QUANTUM_NUMBER_WRITE_G(quantum_number, RDD_COMPLEX_SCHEDULER_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_tm_complex_scheduler_table_quantum_number_get(uint32_t _entry, uint8_t *quantum_number)
{
    if(_entry >= RDD_COMPLEX_SCHEDULER_TABLE_SIZE)
         return BDMF_ERR_PARM;

    RDD_COMPLEX_SCHEDULER_DESCRIPTOR_QUANTUM_NUMBER_READ_G(*quantum_number, RDD_COMPLEX_SCHEDULER_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

