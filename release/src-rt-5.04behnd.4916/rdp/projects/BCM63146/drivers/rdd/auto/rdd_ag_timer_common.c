/*
   Copyright (c) 2015 Broadcom
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License, version 2, as published by
    the Free Software Foundation (the "GPL").
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    
    A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
    writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
    
:>
*/



/* This is an automated file. Do not edit its contents. */


#include "rdd.h"

#include "rdd_ag_timer_common.h"

int rdd_ag_timer_common_cbs_entry_get(uint32_t _entry, rdd_cbs_entry_t *cbs_entry)
{
    if(!cbs_entry || _entry >= RDD_FW_POLICER_CBS_SIZE)
         return BDMF_ERR_PARM;

    RDD_CBS_ENTRY_COMMITTED_BURST_SIZE_READ_G(cbs_entry->committed_burst_size, RDD_FW_POLICER_CBS_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_timer_common_cbs_entry_set(uint32_t _entry, rdd_cbs_entry_t *cbs_entry)
{
    if(!cbs_entry || _entry >= RDD_FW_POLICER_CBS_SIZE)
         return BDMF_ERR_PARM;

    RDD_CBS_ENTRY_COMMITTED_BURST_SIZE_WRITE_G(cbs_entry->committed_burst_size, RDD_FW_POLICER_CBS_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_timer_common_fw_policer_budget_entry_get(uint32_t _entry, rdd_fw_policer_budget_entry_t *fw_policer_budget_entry)
{
    if(!fw_policer_budget_entry || _entry >= RDD_FW_POLICER_BUDGET_SIZE)
         return BDMF_ERR_PARM;

    RDD_FW_POLICER_BUDGET_ENTRY_BUDGET_MANTISSA_READ_G(fw_policer_budget_entry->budget_mantissa, RDD_FW_POLICER_BUDGET_ADDRESS_ARR, _entry);
    RDD_FW_POLICER_BUDGET_ENTRY_BUDGET_EXPONENT_READ_G(fw_policer_budget_entry->budget_exponent, RDD_FW_POLICER_BUDGET_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_timer_common_fw_policer_budget_entry_set(uint32_t _entry, rdd_fw_policer_budget_entry_t *fw_policer_budget_entry)
{
    if(!fw_policer_budget_entry || _entry >= RDD_FW_POLICER_BUDGET_SIZE || fw_policer_budget_entry->budget_mantissa >= 16384 || fw_policer_budget_entry->budget_exponent >= 4)
          return BDMF_ERR_PARM;

    RDD_FW_POLICER_BUDGET_ENTRY_BUDGET_MANTISSA_WRITE_G(fw_policer_budget_entry->budget_mantissa, RDD_FW_POLICER_BUDGET_ADDRESS_ARR, _entry);
    RDD_FW_POLICER_BUDGET_ENTRY_BUDGET_EXPONENT_WRITE_G(fw_policer_budget_entry->budget_exponent, RDD_FW_POLICER_BUDGET_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_timer_common_fw_policer_cbs_committed_burst_size_set(uint32_t _entry, uint32_t committed_burst_size)
{
    if(_entry >= RDD_FW_POLICER_CBS_SIZE)
         return BDMF_ERR_PARM;

    RDD_CBS_ENTRY_COMMITTED_BURST_SIZE_WRITE_G(committed_burst_size, RDD_FW_POLICER_CBS_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_timer_common_fw_policer_cbs_committed_burst_size_get(uint32_t _entry, uint32_t *committed_burst_size)
{
    if(_entry >= RDD_FW_POLICER_CBS_SIZE)
         return BDMF_ERR_PARM;

    RDD_CBS_ENTRY_COMMITTED_BURST_SIZE_READ_G(*committed_burst_size, RDD_FW_POLICER_CBS_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_timer_common_fw_policer_budget_remainder_set(uint32_t _entry, uint8_t bits)
{
    if(_entry >= RDD_FW_POLICER_BUDGET_REMAINDER_SIZE)
          return BDMF_ERR_PARM;

    RDD_BYTE_1_BITS_WRITE_G(bits, RDD_FW_POLICER_BUDGET_REMAINDER_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_timer_common_fw_policer_budget_remainder_get(uint32_t _entry, uint8_t *bits)
{
    if(_entry >= RDD_FW_POLICER_BUDGET_REMAINDER_SIZE)
          return BDMF_ERR_PARM;

    RDD_BYTE_1_BITS_READ_G(*bits, RDD_FW_POLICER_BUDGET_REMAINDER_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_timer_common_fw_policer_budget_budget_mantissa_set(uint32_t _entry, uint16_t budget_mantissa)
{
    if(_entry >= RDD_FW_POLICER_BUDGET_SIZE || budget_mantissa >= 16384)
          return BDMF_ERR_PARM;

    RDD_FW_POLICER_BUDGET_ENTRY_BUDGET_MANTISSA_WRITE_G(budget_mantissa, RDD_FW_POLICER_BUDGET_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_timer_common_fw_policer_budget_budget_mantissa_get(uint32_t _entry, uint16_t *budget_mantissa)
{
    if(_entry >= RDD_FW_POLICER_BUDGET_SIZE)
         return BDMF_ERR_PARM;

    RDD_FW_POLICER_BUDGET_ENTRY_BUDGET_MANTISSA_READ_G(*budget_mantissa, RDD_FW_POLICER_BUDGET_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_timer_common_fw_policer_budget_budget_exponent_set(uint32_t _entry, uint8_t budget_exponent)
{
    if(_entry >= RDD_FW_POLICER_BUDGET_SIZE || budget_exponent >= 4)
          return BDMF_ERR_PARM;

    RDD_FW_POLICER_BUDGET_ENTRY_BUDGET_EXPONENT_WRITE_G(budget_exponent, RDD_FW_POLICER_BUDGET_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_timer_common_fw_policer_budget_budget_exponent_get(uint32_t _entry, uint8_t *budget_exponent)
{
    if(_entry >= RDD_FW_POLICER_BUDGET_SIZE)
         return BDMF_ERR_PARM;

    RDD_FW_POLICER_BUDGET_ENTRY_BUDGET_EXPONENT_READ_G(*budget_exponent, RDD_FW_POLICER_BUDGET_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_timer_common_fw_policer_vector_set(uint32_t _entry, uint32_t bits)
{
    if(_entry >= RDD_FW_POLICER_VECTOR_SIZE)
          return BDMF_ERR_PARM;

    RDD_BYTES_4_BITS_WRITE_G(bits, RDD_FW_POLICER_VECTOR_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_timer_common_fw_policer_vector_get(uint32_t _entry, uint32_t *bits)
{
    if(_entry >= RDD_FW_POLICER_VECTOR_SIZE)
          return BDMF_ERR_PARM;

    RDD_BYTES_4_BITS_READ_G(*bits, RDD_FW_POLICER_VECTOR_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

