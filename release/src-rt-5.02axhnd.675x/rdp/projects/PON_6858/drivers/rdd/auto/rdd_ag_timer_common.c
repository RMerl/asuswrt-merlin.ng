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

int rdd_ag_timer_common_emac_flow_ctrl_vector_set(uint8_t bits)
{
    RDD_BYTE_1_BITS_WRITE_G(bits, RDD_EMAC_FLOW_CTRL_VECTOR_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_timer_common_emac_flow_ctrl_vector_get(uint8_t *bits)
{
    RDD_BYTE_1_BITS_READ_G(*bits, RDD_EMAC_FLOW_CTRL_VECTOR_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_timer_common_emac_flow_ctrl_budget_budget_set(uint32_t _entry, uint32_t budget)
{
    if(_entry >= RDD_EMAC_FLOW_CTRL_BUDGET_SIZE)
         return BDMF_ERR_PARM;

    RDD_EMAC_FLOW_CTRL_BUDGET_ENTRY_BUDGET_WRITE_G(budget, RDD_EMAC_FLOW_CTRL_BUDGET_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_timer_common_emac_flow_ctrl_budget_budget_get(uint32_t _entry, uint32_t *budget)
{
    if(_entry >= RDD_EMAC_FLOW_CTRL_BUDGET_SIZE)
         return BDMF_ERR_PARM;

    RDD_EMAC_FLOW_CTRL_BUDGET_ENTRY_BUDGET_READ_G(*budget, RDD_EMAC_FLOW_CTRL_BUDGET_ADDRESS_ARR, _entry);

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

int rdd_ag_timer_common_emac_flow_ctrl_budget_remainder_set(uint32_t _entry, uint8_t bits)
{
    if(_entry >= RDD_EMAC_FLOW_CTRL_BUDGET_REMAINDER_SIZE)
          return BDMF_ERR_PARM;

    RDD_BYTE_1_BITS_WRITE_G(bits, RDD_EMAC_FLOW_CTRL_BUDGET_REMAINDER_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_timer_common_emac_flow_ctrl_budget_remainder_get(uint32_t _entry, uint8_t *bits)
{
    if(_entry >= RDD_EMAC_FLOW_CTRL_BUDGET_REMAINDER_SIZE)
          return BDMF_ERR_PARM;

    RDD_BYTE_1_BITS_READ_G(*bits, RDD_EMAC_FLOW_CTRL_BUDGET_REMAINDER_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

