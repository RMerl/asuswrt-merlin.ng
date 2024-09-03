/*
   <:copyright-BRCM:2014-2016:DUAL/GPL:standard
   
      Copyright (c) 2014-2016 Broadcom 
      All Rights Reserved
   
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

#ifndef _RDD_INIT_H
#define _RDD_INIT_H

#include "rdd_defs.h"
#include "rdp_drv_bpm.h"
#ifndef LEGACY_RDP
#include "rdd_stubs.h"
#endif
#include "rdd_init_common.h"
#include "rdd_proj_init.h"

int rdd_init(void);
void rdd_exit(void);
int rdd_data_structures_init(rdd_init_params_t *init_params);
void rdd_load_microcode(uint8_t *runner_a_microcode, uint8_t *runner_b_microcode, uint8_t *runner_c_microcode,
    uint8_t *runner_d_microcode);
void rdd_load_prediction(uint8_t *runner_a_predict, uint8_t *runner_b_predict, uint8_t *runner_c_predict,
    uint8_t *runner_d_predict);
void rdd_runner_enable(void);
void rdd_runner_disable(void);
void rdd_runner_frequency_set(uint16_t freq);
void rdd_action_vector_set(uint16_t *action_ptrs_ptr, uint16_t *action_ptrs, uint8_t action_total_num);
void rdd_bpm_ddr_optimized_base_cfg(uint32_t ddr_packet_headroom_size);

#endif /* _RDD_INIT_H */
