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


#ifndef _RDD_AG_TIMER_COMMON_H_
#define _RDD_AG_TIMER_COMMON_H_

#include "rdd.h"

typedef struct
{
    uint16_t budget_mantissa;
    uint8_t budget_exponent;
} rdd_fw_policer_budget_entry_t;

typedef struct
{
    uint32_t committed_burst_size;
} rdd_cbs_entry_t;

int rdd_ag_timer_common_fw_policer_budget_entry_get(uint32_t _entry, rdd_fw_policer_budget_entry_t *fw_policer_budget_entry);
int rdd_ag_timer_common_fw_policer_budget_entry_set(uint32_t _entry, rdd_fw_policer_budget_entry_t *fw_policer_budget_entry);
int rdd_ag_timer_common_cbs_entry_get(uint32_t _entry, rdd_cbs_entry_t *cbs_entry);
int rdd_ag_timer_common_cbs_entry_set(uint32_t _entry, rdd_cbs_entry_t *cbs_entry);
int rdd_ag_timer_common_fw_policer_budget_budget_mantissa_set(uint32_t _entry, uint16_t budget_mantissa);
int rdd_ag_timer_common_fw_policer_budget_budget_mantissa_get(uint32_t _entry, uint16_t *budget_mantissa);
int rdd_ag_timer_common_fw_policer_budget_budget_exponent_set(uint32_t _entry, uint8_t budget_exponent);
int rdd_ag_timer_common_fw_policer_budget_budget_exponent_get(uint32_t _entry, uint8_t *budget_exponent);
int rdd_ag_timer_common_fw_policer_cbs_committed_burst_size_set(uint32_t _entry, uint32_t committed_burst_size);
int rdd_ag_timer_common_fw_policer_cbs_committed_burst_size_get(uint32_t _entry, uint32_t *committed_burst_size);
int rdd_ag_timer_common_fw_policer_budget_remainder_set(uint32_t _entry, uint8_t bits);
int rdd_ag_timer_common_fw_policer_budget_remainder_get(uint32_t _entry, uint8_t *bits);
int rdd_ag_timer_common_emac_flow_ctrl_vector_set(uint8_t bits);
int rdd_ag_timer_common_emac_flow_ctrl_vector_get(uint8_t *bits);
int rdd_ag_timer_common_fw_policer_vector_set(uint32_t _entry, uint32_t bits);
int rdd_ag_timer_common_fw_policer_vector_get(uint32_t _entry, uint32_t *bits);
int rdd_ag_timer_common_emac_flow_ctrl_budget_budget_set(uint32_t _entry, uint32_t budget);
int rdd_ag_timer_common_emac_flow_ctrl_budget_budget_get(uint32_t _entry, uint32_t *budget);
int rdd_ag_timer_common_emac_flow_ctrl_budget_remainder_set(uint32_t _entry, uint8_t bits);
int rdd_ag_timer_common_emac_flow_ctrl_budget_remainder_get(uint32_t _entry, uint8_t *bits);

#endif /* _RDD_AG_TIMER_COMMON_H_ */
