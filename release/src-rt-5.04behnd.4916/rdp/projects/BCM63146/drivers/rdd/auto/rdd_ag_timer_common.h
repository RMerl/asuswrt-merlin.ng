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


#ifndef _RDD_AG_TIMER_COMMON_H_
#define _RDD_AG_TIMER_COMMON_H_

typedef struct rdd_cbs_entry_s
{
    uint32_t committed_burst_size;
} rdd_cbs_entry_t;

typedef struct rdd_fw_policer_budget_entry_s
{
    uint16_t budget_mantissa;
    uint8_t budget_exponent;
} rdd_fw_policer_budget_entry_t;

int rdd_ag_timer_common_cbs_entry_get(uint32_t _entry, rdd_cbs_entry_t *cbs_entry);
int rdd_ag_timer_common_cbs_entry_set(uint32_t _entry, rdd_cbs_entry_t *cbs_entry);
int rdd_ag_timer_common_fw_policer_budget_entry_get(uint32_t _entry, rdd_fw_policer_budget_entry_t *fw_policer_budget_entry);
int rdd_ag_timer_common_fw_policer_budget_entry_set(uint32_t _entry, rdd_fw_policer_budget_entry_t *fw_policer_budget_entry);
int rdd_ag_timer_common_fw_policer_cbs_committed_burst_size_set(uint32_t _entry, uint32_t committed_burst_size);
int rdd_ag_timer_common_fw_policer_cbs_committed_burst_size_get(uint32_t _entry, uint32_t *committed_burst_size);
int rdd_ag_timer_common_fw_policer_budget_remainder_set(uint32_t _entry, uint8_t bits);
int rdd_ag_timer_common_fw_policer_budget_remainder_get(uint32_t _entry, uint8_t *bits);
int rdd_ag_timer_common_fw_policer_budget_budget_mantissa_set(uint32_t _entry, uint16_t budget_mantissa);
int rdd_ag_timer_common_fw_policer_budget_budget_mantissa_get(uint32_t _entry, uint16_t *budget_mantissa);
int rdd_ag_timer_common_fw_policer_budget_budget_exponent_set(uint32_t _entry, uint8_t budget_exponent);
int rdd_ag_timer_common_fw_policer_budget_budget_exponent_get(uint32_t _entry, uint8_t *budget_exponent);
int rdd_ag_timer_common_fw_policer_vector_set(uint32_t _entry, uint32_t bits);
int rdd_ag_timer_common_fw_policer_vector_get(uint32_t _entry, uint32_t *bits);

#endif /* _RDD_AG_TIMER_COMMON_H_ */
